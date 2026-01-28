#include "stdafx.h"
#include "TCPtoRPi.h"
#include <winsock2.h>
#include <ws2tcpip.h>


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



void MessageQueue::push(const std::string& msg)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_queue.push(msg);
	}
	m_cv.notify_one();
}

std::string MessageQueue::pop()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_cv.wait(lock, [this] { return !m_queue.empty(); });

	std::string msg = m_queue.front();
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
// TCP listener routine
// ------------------------------------------------------------

static void PLCListenerWorker(MessageQueue* PLC_out,
	MessageQueue* PLC_in)
{
	g_listenerRunning = true;

	while (g_listenerRunning)
	{
		// ----------------------------------------------------
		// 1. Read from PLC (blocking or polling)
		// Replace this with your actual TCP receive logic
		// ----------------------------------------------------
		std::string incoming = "PONG";   // placeholder

										 // If shutdown was requested, exit cleanly
		if (!g_listenerRunning)
			break;

		// ----------------------------------------------------
		// 2. Push event to main logic
		// ----------------------------------------------------
		PLC_out->push(incoming);

		// ----------------------------------------------------
		// 3. Check if main logic sent a command
		// ----------------------------------------------------
		if (!PLC_in->empty())
		{
			std::string cmd = PLC_in->pop();

			if (cmd == "__STOP__")
				break;

			// Send command to PLC (your TCP send logic)
			// sendToPLC(cmd);
		}
	}
}

// ------------------------------------------------------------
// Start
// ------------------------------------------------------------
void StartPLCListenerThread(MessageQueue& q_out,
	MessageQueue& q_in)
{
	g_listenerThread = std::thread(PLCListenerWorker, &q_out, &q_in);
}

// ------------------------------------------------------------
// Stop
// ------------------------------------------------------------
void StopPLCListenerThread()
{
	g_listenerRunning = false;

	if (g_listenerThread.joinable())
		g_listenerThread.join();
}


static void MainLogicWorker(MessageQueue* PLC_out, MessageQueue* PLC_in){
	g_logicRunning = true;
	while (g_logicRunning){
		// 1. Wait for next PLC event (blocking) 
		std::string event = PLC_out->pop();
		// 2. Process the event (your state machine goes here) 
		// ---------------------------------------------------- 
		// Example placeholder logic: 
		if (event == "START") {
			PLC_in->push("START_MEASUREMENT"); 
		}
		else if (event == "PONG") {
			// to do); 
		} 
		else if (event == "ERROR") {
			PLC_in->push("RESET"); 
		}
		// ---------------------------------------------------- 
	}
}

void StartMainLogicThread(){
	g_logicThread = std::thread(MainLogicWorker, &g_PLC_out, &g_PLC_in);
	StartPLCListenerThread(g_PLC_out, g_PLC_in);
}

void StopMainLogicThread()
{
	StopPLCListenerThread();
	g_logicRunning = false;

	// Unblock pop()
	g_PLC_out.push("__STOP__");

	if (g_logicThread.joinable())
		g_logicThread.join();
}
