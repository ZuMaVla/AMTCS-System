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
		return false;

	sockaddr_in addr{};									// Empty address
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(sock);
		WSACleanup();
		return false;
	}

	send(sock, msg.c_str(), (int)msg.size(), 0);

	char buffer[1024] = {};
	int bytes = recv(sock, buffer, sizeof(buffer), 0);

	if (bytes > 0)
		replyOut.assign(buffer, bytes);

	closesocket(sock);
	WSACleanup();
	return bytes > 0;
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
	g_logicThread = std::thread(MainLogicWorker, &g_PLC_out, &g_PLC_in);
	
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
	freopen("CONOUT$", "w", stdout);

	// -----------------------------
	// 1. Create server socket
	// -----------------------------
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;


	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd < 0) {
		std::cerr << "[TCP] Failed to create socket\n";
		return;
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&opt), sizeof(opt));
	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;     // or fixed IP
	addr.sin_port = htons(5050);           // same as PLC-TCP-listener

	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
		std::cerr << "[TCP] Bind failed\n";
		closesocket(server_fd);
		return;
	}

	if (listen(server_fd, 1) < 0) {
		std::cerr << "[TCP] Listen failed\n";
		closesocket(server_fd);
		return;
	}

	std::cout << "[TCP] Listener running...\n";

	// -----------------------------
	// 2. Main loop
	// -----------------------------
	while (g_listenerRunning)
	{
		// -----------------------------------------
		// Accept a client (blocking with timeout)
		// -----------------------------------------
		fd_set set;
		FD_ZERO(&set);
		FD_SET(server_fd, &set);

		timeval timeout{};
		timeout.tv_sec = 1;   // 1-second poll
		timeout.tv_usec = 0;

		int rv = select(server_fd + 1, &set, nullptr, nullptr, &timeout);

		if (rv < 0) {
			std::cerr << "[TCP] select() error\n";
			break;
		}

		if (rv == 0) {
			// Timeout → check for shutdown or outgoing commands
			if (!PLC_in.empty()) {
				Message cmd = PLC_in.pop();
				if (cmd.keyword == "STATUS" && cmd.payload == "__STOP__")
					g_listenerRunning = false;
			}
			continue;
		}

		// -----------------------------------------
		// Client connected
		// -----------------------------------------
		sockaddr_in client_addr{};
		socklen_t client_len = sizeof(client_addr);

		int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
		if (client_fd < 0) {
			std::cerr << "[TCP] Accept failed\n";
			continue;
		}

		// -----------------------------------------
		// Receive message
		// -----------------------------------------
		char buffer[1024] = { 0 };
		int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

		if (bytes > 0) {
			std::string msg(buffer);
			Message evt;

			// Parse keyword + payload
			auto pos = msg.find(' ');
			if (pos == std::string::npos) {
				evt.keyword = msg;
				evt.payload = "";
			}
			else {
				evt.keyword = msg.substr(0, pos);
				evt.payload = msg.substr(pos + 1);
			}

			PLC_out.push(evt);
		}

		// -----------------------------------------
		// Check if logic thread sent a command
		// -----------------------------------------
		if (!PLC_in.empty()) {
			Message cmd = PLC_in.pop();

			std::string out = cmd.keyword + " " + cmd.payload;
			send(client_fd, out.c_str(), out.size(), 0);

			if (cmd.keyword == "STATUS" && cmd.payload == "__STOP__")
				g_listenerRunning = false;
		}

		closesocket(client_fd);
	}

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



