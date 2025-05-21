#include <iostream>

#include "Socket.h"

Socket::Socket() : m_Socket(INVALID_SOCKET), m_LpfnAcceptEx(nullptr){}
Socket::~Socket() { 
	CloseSocket();
}

Socket::Socket(Socket&& other) noexcept
	: m_Socket(other.m_Socket),
	m_LpfnAcceptEx(other.m_LpfnAcceptEx) {
	other.m_Socket = INVALID_SOCKET;
	other.m_LpfnAcceptEx = nullptr;
}


Socket& Socket::operator=(Socket&& other) noexcept {
	if (this != &other) {
		if (m_Socket != INVALID_SOCKET) {
			closesocket(m_Socket);

			m_Socket = other.m_Socket;
			m_LpfnAcceptEx = other.m_LpfnAcceptEx;
		}

		other.m_Socket = INVALID_SOCKET;
		other.m_LpfnAcceptEx = nullptr;
	}
	return *this;
}

bool Socket::SocketInit(ProtocolType type) {
	int sockType = 0;
	int protocol = 0;

	if (type == ProtocolType::TCP) {
		sockType = SOCK_STREAM;
		protocol = IPPROTO_TCP;
	} else {
		sockType = SOCK_DGRAM;
		protocol = IPPROTO_UDP;
	}

	m_Socket = WSASocket(AF_INET, sockType, protocol, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (m_Socket == INVALID_SOCKET) {
		std::cout << "WSASocket failed with error: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool Socket::SocketBind(int port) {
	sockaddr_in serverAddr;
	if (!InitSockAddr(serverAddr, nullptr, port))
		return false;

	if (bind(m_Socket,
		reinterpret_cast<SOCKADDR*>(&serverAddr),
		sizeof(serverAddr)) == SOCKET_ERROR)
	{
		CloseSocket();
		std::cout << "SocketBind failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

// backLog = 커널이 관리하는 연결 큐의 최대 길이
bool Socket::SocketListen(int backLog) {
	return SOCKET_ERROR != listen(m_Socket, backLog);
}

void Socket::CloseSocket() {
	if (m_Socket != INVALID_SOCKET) {
		closesocket(m_Socket);
		m_Socket = INVALID_SOCKET;
	}
}


bool Socket::Connect(const char* ip, int port) {
	sockaddr_in servAddr;
	if (!InitSockAddr(servAddr, ip, port))
		return false;

	if (connect(m_Socket,
		reinterpret_cast<SOCKADDR*>(&servAddr),
		sizeof(servAddr)) == SOCKET_ERROR)
	{
		std::cout << "Connect failed: " << WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool Socket::LoadAcceptExFunc() {
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

bool Socket::AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer) {
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

bool Socket::SetAcceptContext(Socket& listenSocket) {
	SOCKET s = listenSocket.GetSocket();
	if (setsockopt(
		m_Socket,
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<const char*>(&s),
		sizeof(SOCKET)) == SOCKET_ERROR) 
	{
		std::cout << "setsockopt failed with error: " << WSAGetLastError() << std::endl;
		return false;
	}

	return true;
}

bool Socket::InitSockAddr(sockaddr_in& outAddr, const char* ip, int port) {
	ZeroMemory(&outAddr, sizeof(outAddr));
	outAddr.sin_family = AF_INET;
	outAddr.sin_port = htons(port);

	if (ip) {
		int rv = inet_pton(AF_INET, ip, &outAddr.sin_addr);
		if (rv != 1) {
			std::cout << "Invalid IP format: " << ip << std::endl;
			return false;
		}
	} else {
		outAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}
	return true;
}

