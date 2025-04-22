#include "IOCPServer.h"
#include <iostream>
#include <WinSock2.h>

IOCPserver::IOCPserver() : m_listenSocket(INVALID_SOCKET) {}
IOCPserver::~IOCPserver() {}

bool IOCPserver::SocketInit()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Fail WSA init" << std::endl;
		return false;
	}

	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (m_listenSocket == INVALID_SOCKET) {
		std::cout << "WSASocket failed with error: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool IOCPserver::SocketBind(int port)
{
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	int result = bind(m_listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR) {
		closesocket(m_listenSocket);
		std::cout << "Fail Socket Bind" << std::endl;
		return false;
	}

	return true;
}

bool IOCPserver::SocketListen(int backLog)
{
	return SOCKET_ERROR != listen(m_listenSocket, backLog);
}
