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
