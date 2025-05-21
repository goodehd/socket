#include <iostream>
#include "IOCPListener.h"

IOCPListener::IOCPListener(unsigned short port, IOCPserver owner, AcceptHandler handler)
	:m_port(port),
	 m_owner(owner),
	 m_handler(handler){}

bool IOCPListener::InitListener() {
	if (!m_listenSocket.SocketInit(ProtocolType::TCP)) {
		return false;
	}

	if (!m_listenSocket.SocketBind(m_port)) {
		return false;
	}

	if (!m_listenSocket.SocketListen(SOMAXCONN)) {
		std::cout << "Fail Socket listen" << std::endl;
		return false;
	}

	m_owner.IocpAdd(m_listenSocket.GetSocket(), nullptr);

	if (!LoadAcceptExFunc()) {
		return false;
	}

	return true;
}

bool IOCPListener::AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer) {
	DWORD bytesReceived = 0;

	BOOL result = m_lpfnAcceptEx(
		m_listenSocket.GetSocket(),
		clientSocket.GetSocket(),
		buffer,
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&bytesReceived,
		overlapped
	);

	if (result == FALSE) {
		int err = WSAGetLastError();
		if (err != ERROR_IO_PENDING) {
			std::cout << "AcceptEx failed with error: " << err << std::endl;
			return false;
		}
	}

	return true;
}

bool IOCPListener::LoadAcceptExFunc() {
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;

	return WSAIoctl(
		m_listenSocket.GetSocket(),
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&bytes,
		NULL,
		NULL) != SOCKET_ERROR;
}