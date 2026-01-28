#pragma once
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <queue>




class MessageQueue{
public: void push(const std::string& msg);
		std::string pop();
		bool empty() const;
private: mutable std::mutex m_mutex;
		 std::condition_variable m_cv;
		 std::queue<std::string> m_queue; 
};

// Main logic threads 
void StartMainLogicThread();
void StopMainLogicThread();

// TCP listener threads
void StartPLCListenerThread(MessageQueue& out_queue, MessageQueue& in_queue);
void StopPLCListenerThread();
bool SendTCPMessage(const std::string& ip, int port, const std::string& msg, std::string& replyOut);