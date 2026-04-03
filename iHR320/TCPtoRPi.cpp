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

static void MainLogicWorker(CiHR320Dlg *pUI, MessageQueue &PLC_out, MessageQueue &PLC_in){

	std::string localIP = pUI->GetLocalIP();					// (!!!) retrieving IP address of self from UI

	StartPLCListenerThread(pUI, localIP, port_iHR320, PLC_out, PLC_in);
	g_logicRunning = true;
	bool isMeasuring = false;
	Message cmd, event;
	CString msg;

	while (g_logicRunning){

		// Wait for next PLC event (blocking, but not longer than HBRate seconds) 
		event = {"", ""};										// 
		PLC_out.popTimed(event, HBRate*100);

		// Process the event (event interpretation) 

		if (event.keyword == "PONG") {												// PLC response - alive
			std::cout << "[PLC] I am alive\n"; 
			auto* pDevice = new std::string("PLC");									// create pointer to a string containing "PLC"
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
			cmd.keyword = "SEND";
			cmd.payload = "TC?";
			PLC_in.push(cmd); 														// Request status of TC
		}
		else if (event.keyword == "EXP_CONFIRMED") {								// PLC confirmed experiment state				
			msg = _T("Experiment details accepted");
			pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, _T("EDA: ") + msg);			// Message to main window
		}
		else if (event.keyword == "EXP_ERROR") {									// PLC confirmed experiment state				
			msg = _T("Experiment details failed: ") + CString(event.payload.c_str());
			pUI->PostMessageToUI(
				WM_USER_LOG_MESSAGE, _T("EDF: ") + msg);							// Message to main window
		}
		else if (event.keyword == "EXP_NONE") {										// PLC reported no experiment 				
			pUI->PostMessageToUI(
				WM_USER_LOG_MESSAGE, CString(event.keyword.c_str()));				// Message to main window
		}
		else if (event.keyword == "EXP:") {											// PLC sent experiment state				
			pUI->jsonState = event.payload;
			Sleep(100);
			std::cout << "[PLC] Exp State: " << event.payload << "\n";
			pUI->PostMessageToUI(
				WM_USER_LOG_MESSAGE, _T("EXP_ON"));									// Message to main window to deal with received exp state
		}
		else if (event.keyword == "ACQUIRE_SPECTRUM") {								// Spectrum acquisition requested				
			if (pUI->m_isExperimentStarted) {
				SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, 2);		// turn off monitors
				cmd.keyword = "SEND";
				cmd.payload = "AFFIRMATIVE";
				PLC_in.push(cmd); 													// Confirming request
				msg = CString(event.payload.c_str());
				pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, L"CT= " + msg);			// Current temperature + T itself (msg)
				Sleep(10000);														// To let monitors to turn of completely
				if (TakeSpectrum(pUI, msg)) {
					cmd.payload = "DONE";
					pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, L"SAd");				// Spectrum acquired (SAd)
				}
				else { cmd.payload = "ERROR_SPECTRUM"; }
				PLC_in.push(cmd);													// Confirming success/error
				SendMessage(HWND_BROADCAST, WM_SYSCOMMAND, SC_MONITORPOWER, -1);	// turn monitors back
			}
			else {
				cmd.keyword = "SEND";
				cmd.payload = "ERROR_SPECTRUM";
				PLC_in.push(cmd); 													// Refusing request (reporting error)
			}
		}
		else if (event.keyword == "TC_OK") {										// PLC response - TC alive
			std::cout << "[PLC] TC alive\n";
			auto* pDevice = new std::string(event.keyword);							// create pointer to a string containing device/status
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
		}
		else if (event.keyword == "TC_OFF") {										// PLC response - TC not connected
			std::cout << "[PLC] TC not responding\n";
			auto* pDevice = new std::string(event.keyword);							// create pointer to a string containing device/status
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
		}
		else if (event.keyword == "TC_READY") {										// PLC response - TC ready
			std::cout << "[PLC] TC ready\n";
			auto* pDevice = new std::string(event.keyword);							// create pointer to a string containing device/status
			pUI->PostMessage(
				WM_UPDATE_SYSTEM_STATUS,
				0,
				reinterpret_cast<LPARAM>(pDevice)
			);
		}
		else if (event.keyword == "T=") {											// Incoming current T from PLC/TC (init)
			std::cout << "[TC] Current T: " + event.payload + " K\n";
			CString msg = CString(event.keyword.c_str());
			msg += event.payload.c_str();											// Converting event to CString
			pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, msg);
		}
		else if (event.keyword == "TARGET_T=") {									// Incoming target T from PLC/TC (exp)
			std::cout << "[TC] New target: " + event.payload + " K\n";
			CString msg = CString(event.keyword.c_str());
			msg += event.payload.c_str();											// Converting event to CString
			pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, msg);
		}
		else if (event.keyword == "EVENT" && event.payload == "__STOP__") {			// User requested abort 
			cmd.keyword = "SEND";
			cmd.payload = "OFF";
			PLC_in.push(cmd);
		}
		else if (event.keyword == "REQUEST" && event.payload == "PLC_STATUS") {		// User requested to ping PLC 
			cmd.keyword = "SEND";
			cmd.payload = "PING";														 
			PLC_in.push(cmd);

		}
		else if (event.keyword == "REQUEST" && event.payload == "EXP_STATUS?") {	// PLC requested experiment status
			pUI->m_missedPLCHeartbeatCount = 0;
			pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, _T("PLC_OK"));
			if (pUI->m_isExperimentStarted) {
				cmd.keyword = "SEND";
				cmd.payload = "EXP_STATUS RUNNING";
			}
			else {
				cmd.keyword = "SEND";
				cmd.payload = "EXP_STATUS NOT_STARTED";
			}
			PLC_in.push(cmd);
			std::cout << "PLC has been informed: " + cmd.payload + "\n";
		}
		else if (event.keyword == "REQUEST" && event.payload == "EXP_STATE?") {		// PLC requested experiment state
			CString msg = _T("RECOVER_EXP");
			pUI->PostMessageToUI(WM_UPDATE_SYSTEM_EVENT, msg);
		}
		else if (event.keyword == "CONFIRM_PAUSE_CONTINUE") {						// PLC confirmed suspend/resume command
			pUI->StopTimer(TIMER_EXP_PAUSE_CONTINUE);
		}
		else if (event.keyword == "CONFIRM_CANCEL") {								// PLC confirmed cancellation of current exp
			pUI->StopTimer(TIMER_EXP_CANCEL);
		}
		else if (event.keyword == "CONFIRM_OFF") {									// PLC confirmed turning off
			pUI->m_isPLCConfirmedOff = true;
			cmd.keyword = "OFF";
			cmd.payload = "";
			PLC_in.push(cmd);
			g_logicRunning = false;
			pUI->StopTimer(TIMER_PLC_STOP);
		}
		else if (event.keyword == "EXPERIMENT_FINISHED") {							// PLC informed about the end of experiment
			std::cout << "Experiment finished\n";
			CString msg = _T("EXP_END");
			pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, msg);
		}
		else if (event.keyword == "") {												// If no event, sending heartbeat to PLC
			if (pUI->m_connectivityDlg.m_CheckBoxPLC.GetCheck()) {
				cmd.keyword = "SEND";
				cmd.payload = "ALIVE?";
				PLC_in.push(cmd);
				pUI->m_missedPLCHeartbeatCount++;
				if (pUI->m_missedPLCHeartbeatCount >= 3) {
					pUI->PostMessageToUI(WM_USER_LOG_MESSAGE, _T("RESET_PLC"));
				}
			}
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

static void PLCListenerWorker(CiHR320Dlg *pUI, std::string ip_iHR320, int port_iHR320, MessageQueue& PLC_out, MessageQueue& PLC_in)
{

	g_listenerRunning = true;

	SOCKET server_fd = StartTCPListener(ip_iHR320, port_iHR320);
	int HBRCount = 0;

	// MAIN LOOP (fire-and-forget style communication)

	while (g_listenerRunning)
	{
		// ----- allow graceful stop from PLC_in -----
		if (!PLC_in.empty()) {
			Message cmd = PLC_in.pop();
			if (cmd.keyword == "SEND") {
				std::cout << "[UI-APP] Message to PLC: " << cmd.payload << "\n";
				SendTCPMessage(pUI, ip_PLC, port_PLC, cmd.payload);
			}
			else if (cmd.keyword == "OFF") g_listenerRunning = false;
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
			std::cerr << "[TCP] accept failed: " << WSAGetLastError() << "\n";
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
		std::cout << "[TCP] Listener received: " << fullMsg << "\n";
		if (evt.keyword != "YES") PLC_out.push(evt);	
		else pUI->m_missedPLCHeartbeatCount = 0;			// Reset missed heartbeat count
	}

	closesocket(server_fd);

	std::cout << "[TCP] Listener stopped\n";
}



// ------------------------------------------------------------
// TCP Listener Starter/Stopper
// ------------------------------------------------------------

void StartPLCListenerThread(CiHR320Dlg *pUI, std::string ip_iHR320, int port_iHR320, MessageQueue& queue_out, MessageQueue& queue_in)
{
	g_listenerThread = std::thread(PLCListenerWorker, pUI, ip_iHR320, port_iHR320, std::ref(queue_out), std::ref(queue_in));
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

bool MessageQueue::popTimed(Message & out, int timeout_ms)
{
	std::unique_lock<std::mutex> lock(m_mutex);
	if (!m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this] { return !m_queue.empty(); })) // wait until queue is not empty or timeout expire
	{
		return false; // Timeout occured, mo message available
	}
	out = m_queue.front();
	m_queue.pop();
	return true;
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


bool SendTCPMessage(CiHR320Dlg *pUI, std::string ip, int port, const std::string& msg)
{
	CString timerStr = CString(msg.c_str());

	if (timerStr == _T("REQUEST PLC_STATUS")) pUI->StartTimer(TIMER_PLC_CHECK, 10);
	else if (timerStr.Left(4) == _T("INIT")) pUI->StartTimer(TIMER_EXP_SENT, 10);
	else if (timerStr == _T("EVENT __STOP__")) pUI->StartTimer(TIMER_PLC_STOP, 10);
	else if (timerStr == _T("PAUSE")) pUI->StartTimer(TIMER_EXP_PAUSE_CONTINUE, 5);
	else if (timerStr == _T("CONTINUE")) pUI->StartTimer(TIMER_EXP_PAUSE_CONTINUE, 5);
	else if (timerStr == _T("CANCEL")) pUI->StartTimer(TIMER_EXP_CANCEL, 5);
	else if (timerStr == _T("EXPERIMENT?")) pUI->StartTimer(TIMER_EXP_REQUESTED_BY_UI, 10);

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



