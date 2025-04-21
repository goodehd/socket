#include "IOCPServer.h"
#include <iostream>
#include <WinSock2.h>

IOCPserver::IOCPserver() : m_listenSocket(NULL) {}
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

void IOCPserver::SocketBind()
{

}

void IOCPserver::SocketListen()
{

}
