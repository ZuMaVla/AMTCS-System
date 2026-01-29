#pragma once
#include <thread>
#include <atomic>
#include <string>
#include <mutex>
#include <queue>


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
void StartMainLogicThread();
void StopMainLogicThread();

// TCP listener threads
void StartPLCListenerThread(MessageQueue& queue_out, MessageQueue& queue_in);
void StopPLCListenerThread();
bool SendTCPMessage(const std::string& ip, int port, const std::string& msg, std::string& replyOut);