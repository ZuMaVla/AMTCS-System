#pragma once
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <queue>

class CiHR320Dlg;


const std::string ip_PLC = "192.168.50.1";
const int port_PLC = 5050;
const int port_iHR320 = 5051;


struct Message { 
	std::string keyword;
	std::string payload;
};

class MessageQueue{
public: void push(const Message& msg);
		Message pop();
		bool empty() const;
private: mutable std::mutex m_mutex;
		 std::condition_variable m_cv;
		 std::queue<Message> m_queue; 
};

// Main logic threads 
void StartMainLogicThread(LPVOID pParam);
void StopMainLogicThread();

// TCP listener threads
void StartPLCListenerThread(CiHR320Dlg *pUI, const std::string ip, int port, MessageQueue& queue_out, MessageQueue& queue_in);
void StopPLCListenerThread();
bool SendTCPMessage(CiHR320Dlg *pUI, std::string ip, int port, const std::string& msg);
SOCKET StartTCPListener(std::string ip, int port);