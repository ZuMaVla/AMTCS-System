#pragma once
#include <string> 

bool SendTCPMessage(const std::string& ip, int port, const std::string& msg, std::string& replyOut);