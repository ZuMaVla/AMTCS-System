
#include "stdafx.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include "TCPtoRPi.h"
#include "DataAcquisition.h"
#include "iHR320Dlg.h"





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

static void MainLogicWorker(CiHR320Dlg* pUI, MessageQueue& PLC_out, MessageQueue& PLC_in){

	std::string localIP = pUI->GetLocalIP();					// (!!!) retrieving IP address of self from UI

	StartPLCListenerThread(localIP, port_iHR320, PLC_out, PLC_in);
	g_logicRunning = true;
	bool isMeasuring = false;
	Message cmd;
	while (g_logicRunning){

		// 1. Wait for next PLC event (blocking) 
		Message event = PLC_out.pop();


		// 2. Process the event (event interpretation) 

		if (event.keyword == "ACQUIRE_SPECTRUM") {									// Spectrum acquisition requested				
			cmd.keyword = "SEND";
			cmd.payload = "AFFIRMATIVE";			
			PLC_in.push(cmd); 														// Confirming request
			if (TakeSpectrum(pUI)) { cmd.payload = "DONE";	}
			else { cmd.payload = "ERROR"; }
			PLC_in.push(cmd);														// Confirming success/error
		}
		else if (event.keyword == "PONG") {											// PLC response - alive
			std::cout << "PLC alive\n"; 
			auto* pDevice = new std::string("PLC");									// create pointer to a string containing "PLC"
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
			cmd.keyword = "SEND";
			cmd.payload = "TC?";
			PLC_in.push(cmd); 														// Confirming request
		} 
		else if (event.keyword == "TC_OK") {										// PLC response - TC alive
			std::cout << "TC alive\n";
			auto* pDevice = new std::string(event.keyword);							// create pointer to a string containing device/status
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
		}
		else if (event.keyword == "TC_OFF") {										// PLC response - TC not connected
			std::cout << "TC not responding\n";
			auto* pDevice = new std::string(event.keyword);							// create pointer to a string containing device/status
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
		}
		else if (event.keyword == "TC_READY") {										// PLC response - TC ready
			std::cout << "TC ready\n";
			auto* pDevice = new std::string(event.keyword);							// create pointer to a string containing device/status
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
		}
		else if (event.keyword == "T=") {											// Incoming current T from PLC/TC
			std::cout << "Current T: " + event.payload + " K\n";
			CString msg = CString(event.keyword.c_str());
			msg	+= event.payload.c_str();											// Converting event to CString
			pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, msg);
		}
		else if (event.keyword == "STATUS" && event.payload == "MEASUREMENT") {		// PLC informed about being in the middle of experiment sequence
			isMeasuring = true;
			cmd.keyword = "SEND";
			cmd.payload = "GIVE_ME_DETAILS";										// Request for the current status of experiment 
			PLC_in.push(cmd);
		}
		else if (event.keyword == "EVENT" && event.payload == "__STOP__") {			// User requested abort 
			g_logicRunning = false;
		}
		else if (event.keyword == "REQUEST" && event.payload == "PLC_STATUS") {		// User requested to ping PLC 
			cmd.keyword = "SEND";
			cmd.payload = "PING";														 
			PLC_in.push(cmd);

		}
		else if (event.keyword == "REQUEST" && event.payload == "TC_STATUS") {
			cmd.keyword = "SEND";
			cmd.payload = "TC?";
			PLC_in.push(cmd);
		}

	}
}

// ------------------------------------------------------------
// TCP Logic Starter/Stopper
// ------------------------------------------------------------

void StartMainLogicThread(LPVOID pParam){
	CiHR320Dlg* pUI = reinterpret_cast<CiHR320Dlg*>(pParam);
	g_logicThread = std::thread(MainLogicWorker, pUI, std::ref(g_PLC_out), std::ref(g_PLC_in));
	
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

static void PLCListenerWorker(std::string ip_iHR320, int port_iHR320, MessageQueue& PLC_out, MessageQueue& PLC_in)
{

	g_listenerRunning = true;

	SOCKET server_fd = StartTCPListener(ip_iHR320, port_iHR320);
	
	// MAIN LOOP (fire-and-forget style communication)

	while (g_listenerRunning)
	{
		// ----- allow graceful stop from PLC_in -----
		if (!PLC_in.empty()) {
			Message cmd = PLC_in.pop();
			if (cmd.keyword == "STATUS" && cmd.payload == "__STOP__")
				break;
			else if (cmd.keyword == "SEND") {
				SendTCPMessage(ip_PLC, port_PLC, cmd.payload);
			}
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

	closesocket(server_fd);

	std::cout << "[TCP] Listener stopped\n";
}



// ------------------------------------------------------------
// TCP Listener Starter/Stopper
// ------------------------------------------------------------

void StartPLCListenerThread(std::string ip_iHR320, int port_iHR320, MessageQueue& queue_out, MessageQueue& queue_in)
{
	g_listenerThread = std::thread(PLCListenerWorker, ip_iHR320, port_iHR320, std::ref(queue_out), std::ref(queue_in));
}

void StopPLCListenerThread()
{
	g_listenerRunning = false;

	if (g_listenerThread.joinable())
		g_listenerThread.join();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// support implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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


SOCKET StartTCPListener(std::string ip, int port) {

	// Create socket

	SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == INVALID_SOCKET) {
		std::cerr << "socket failed: " << WSAGetLastError() << "\n";
		return server_fd;
	}

	int opt = 1;
	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
		(const char*)&opt, sizeof(opt));

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	inet_pton(addr.sin_family, ip.c_str(), &addr.sin_addr);
	addr.sin_port = htons(port);

	if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		std::cerr << "bind failed: " << WSAGetLastError() << "\n";
		closesocket(server_fd);
		return server_fd;
	}

	if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
		std::cerr << "listen failed: " << WSAGetLastError() << "\n";
		closesocket(server_fd);
		return server_fd;
	}

	std::cout << "[TCP] Listening on port 5051...\n";
	return server_fd;
}


bool SendTCPMessage(std::string ip, int port, const std::string& msg)
{

	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);		// Create socket using IPv4 and TCP
	if (sock == INVALID_SOCKET)
	{
		std::cout << "Socket creation error: " << WSAGetLastError() << "\n";
		return false;
	}

	sockaddr_in addr{};									// Empty address
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
		closesocket(sock);
		std::cout << "Connection error: " << WSAGetLastError() << "\n";

		return false;
	}

	send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);

	closesocket(sock);
	return true;
}



