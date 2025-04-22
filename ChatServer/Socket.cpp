#include "Socket.h"
#include <iostream>

Socket::Socket() : m_Socket(INVALID_SOCKET), m_LpfnAcceptEx(nullptr), m_OverlappedStruct() {}
Socket::~Socket() { 
	CloseSocket();
}

bool Socket::SocketInit()
{
	m_Socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (m_Socket == INVALID_SOCKET) {
		std::cout << "WSASocket failed with error: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool Socket::SocketBind(int port)
{
	SOCKADDR_IN serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(port);

	int result = bind(m_Socket, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
	if (result == SOCKET_ERROR) {
		CloseSocket();
		std::cout << "Fail Socket Bind" << std::endl;
		return false;
	}

	return true;
}

bool Socket::SocketListen(int backLog)
{
	return SOCKET_ERROR != listen(m_Socket, backLog);
}

void Socket::CloseSocket()
{
	if (m_Socket != INVALID_SOCKET) {
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}

bool Socket::ReceiveOverlapped()
{
	int ret = WSARecv(
		m_Socket,
		&m_OverlappedStruct.WsaBuf,
		1,
		NULL,
		&m_OverlappedStruct.LappedFlag,
		&m_OverlappedStruct.ReadOverlappedStruct,
		NULL
	);

	if (ret == SOCKET_ERROR) {
		int err = WSAGetLastError();
		if (err != WSA_IO_PENDING) {
			std::cout << "WSARecv failed: " << err << std::endl;
			return false;
		}
	}

	return true;
}

bool Socket::LoadAcceptExFunc()
{
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;

	 return WSAIoctl(
		m_Socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&m_LpfnAcceptEx,
		sizeof(m_LpfnAcceptEx),
		&bytes,
		NULL,
		NULL) != SOCKET_ERROR;
}

bool Socket::AcceptExSocket(Socket clientSocket, OVERLAPPED* overlapped, char* buffer, DWORD bufferLen)
{
	if (!LoadAcceptExFunc()) {
		std::cout << "Fail LoadAccptEx not load" << std::endl;
		return false;
	}

	DWORD bytesReceived = 0;

	BOOL result = m_LpfnAcceptEx(
		m_Socket,
		clientSocket.GetSocket(),
		buffer,
		0,
		sizeof(SOCKADDR_IN) + 16,
		sizeof(SOCKADDR_IN) + 16,
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

bool Socket::SetAcceptContext(SOCKET listenSocket) {
	if (setsockopt(
		m_Socket,
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		(const char*)&listenSocket,
		sizeof(SOCKET)) == SOCKET_ERROR) 
	{
		std::cout << "setsockopt failed with error: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

