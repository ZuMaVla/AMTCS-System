#include "stdafx.h"
#include "TCPtoRPi.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include "DataAcquisition.h"
#include <iostream>




#pragma comment(lib, "ws2_32.lib")

bool SendTCPMessage(const std::string& ip, int port, const std::string& msg, std::string& replyOut)
{
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return false;

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);		// Create socket using IPv4 and TCP
	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}

	sockaddr_in addr{};									// Empty address
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(sock);
		WSACleanup();
		return false;
	}

	send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);

	closesocket(sock);
	WSACleanup();
	return true;
}



void MessageQueue::push(const Message& msg)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(msg);
	}
	m_cv.notify_one();
}

Message MessageQueue::pop()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cv.wait(lock, [this] { return !m_queue.empty(); });

	Message msg = m_queue.front();
	m_queue.pop();
	return msg;
}

bool MessageQueue::empty() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_queue.empty();
}


static MessageQueue g_PLC_in; 
static MessageQueue g_PLC_out; 
// Main Thread 
static std::thread g_logicThread;
static std::atomic<bool> g_logicRunning{ false };
// PLC Listener Thread
static std::thread g_listenerThread;
static std::atomic<bool> g_listenerRunning{ false };

// ------------------------------------------------------------
// TCP logic routine
// ------------------------------------------------------------

static void MainLogicWorker(MessageQueue& PLC_out, MessageQueue& PLC_in){
	StartPLCListenerThread(PLC_out, PLC_in);
	g_logicRunning = true;
	bool isMeasuring = false;
	Message cmd;
	while (g_logicRunning){

		// 1. Wait for next PLC event (blocking) 
		Message event = PLC_out.pop();


		// 2. Process the event (event interpretation) 

		if (event.keyword == "START") {
			cmd.keyword = "SEND";
			cmd.payload = "AFFIRMATIVE";
			PLC_in.push(cmd); 
			if (TakeSpectrum()) { cmd.payload = "DONE";	}
			else { cmd.payload = "ERROR"; }
			PLC_in.push(cmd);
		}
		else if (event.keyword == "PONG") {
			std::cout << "PLC alive"; 
		} 
		else if (event.keyword == "STATUS" && event.payload == "AFFIRMATIVE") {
			isMeasuring = true;
		}
		else if (event.keyword == "EVENT" && event.payload == "__STOP__") {
			g_logicRunning = false;
		}

	}
}

// ------------------------------------------------------------
// TCP Logic Starter/Stopper
// ------------------------------------------------------------

void StartMainLogicThread(){
	g_logicThread = std::thread(MainLogicWorker, std::ref(g_PLC_out), std::ref(g_PLC_in));
	
}

void StopMainLogicThread()
{
	StopPLCListenerThread();
	g_logicRunning = false;

	// Unblock pop()
	Message cmd;
	cmd.keyword = "EVENT";
	cmd.payload = "__STOP__";
	g_PLC_out.push(cmd);

	if (g_logicThread.joinable())
		g_logicThread.join();
}


// ------------------------------------------------------------
// TCP listener routine
// ------------------------------------------------------------

static void PLCListenerWorker(MessageQueue& PLC_out, MessageQueue& PLC_in)
{
	g_listenerRunning = true;

	AllocConsole();
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);
	freopen_s(&fp, "CONIN$", "r", stdin);

	// -----------------------------
	// Winsock init
	// -----------------------------
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		std::cerr << "WSAStartup failed\n";
		return;
	}

	// -----------------------------
	// Create socket
	// -----------------------------
	SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == INVALID_SOCKET) {
		std::cerr << "socket failed: " << WSAGetLastError() << "\n";
		return;
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;   // listen all interfaces
	addr.sin_port = htons(5051);

	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "bind failed: " << WSAGetLastError() << "\n";
		closesocket(server_fd);
		return;
	}

	if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "listen failed: " << WSAGetLastError() << "\n";
		closesocket(server_fd);
		return;
	}

	std::cout << "[TCP] Listening on port 5051...\n";

	// =====================================================
	// MAIN LOOP (fire-and-forget style)
	// =====================================================
	while (g_listenerRunning)
	{
		// ----- allow graceful stop from PLC_in -----
		if (!PLC_in.empty()) {
			Message cmd = PLC_in.pop();
			if (cmd.keyword == "STATUS" && cmd.payload == "__STOP__")
				break;
		}

		// ----- wait for connection (1s poll) -----
		fd_set set;
		FD_ZERO(&set);
		FD_SET(server_fd, &set);

		timeval timeout{ 1, 0 };

		int rv = select(0, &set, nullptr, nullptr, &timeout);
		if (rv <= 0)
			continue;

		// ----- accept client -----
		sockaddr_in client_addr{};
		int client_len = sizeof(client_addr);

		SOCKET client_fd = accept(server_fd,
			(sockaddr*)&client_addr,
			&client_len);

		if (client_fd == INVALID_SOCKET) {
			std::cerr << "accept failed: " << WSAGetLastError() << "\n";
			continue;
		}

		// ----- receive data (client sends then closes) -----
		char buffer[1024];
		std::string fullMsg;

		while (true)
		{
			int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

			if (bytes <= 0)
				break;

			fullMsg.append(buffer, bytes);
		}

		closesocket(client_fd);

		if (fullMsg.empty())
			continue;

		// ----- parse message -----
		Message evt;

		auto pos = fullMsg.find(' ');
		if (pos == std::string::npos) {
			evt.keyword = fullMsg;
			evt.payload = "";
		}
		else {
			evt.keyword = fullMsg.substr(0, pos);
			evt.payload = fullMsg.substr(pos + 1);
		}

		PLC_out.push(evt);

		std::cout << "[TCP] received: " << fullMsg << "\n";
	}

	// -----------------------------
	// Cleanup
	// -----------------------------
	closesocket(server_fd);
	WSACleanup();

	std::cout << "[TCP] Listener stopped\n";
}



// ------------------------------------------------------------
// TCP Listener Starter/Stopper
// ------------------------------------------------------------

void StartPLCListenerThread(MessageQueue& queue_out, MessageQueue& queue_in)
{
	g_listenerThread = std::thread(PLCListenerWorker, std::ref(queue_out), std::ref(queue_in));
}

void StopPLCListenerThread()
{
	g_listenerRunning = false;

	if (g_listenerThread.joinable())
		g_listenerThread.join();
}



