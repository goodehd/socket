#pragma once

#include <unordered_map>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <memory>
#include <ws2tcpip.h>

#include "Structs.h"

#pragma comment(lib, "ws2_32.lib")

enum class ProtocolType {
	TCP,
	UDP
};

class Socket {

private:
	SOCKET m_Socket;

public :
	Socket();
	~Socket();
	Socket(Socket&& other) noexcept;
	Socket& operator=(Socket&& other) noexcept;
	Socket(const Socket&) = delete;
	Socket& operator=(const Socket&) = delete;

public :
	SOCKET GetSocket() const {
		return m_Socket;
	}

	void SetSocket(SOCKET socket) {
		m_Socket = socket;
	}

public:
	bool SocketInit(ProtocolType type);
	bool SocketBind(int port);
	bool SocketListen(int backLog);
	bool Connect(const char* ip, int port);

	bool AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer);
	bool SetAcceptContext(Socket& listenSocket);
	void CloseSocket();

private :
	bool InitSockAddr(sockaddr_in& outAddr, const char* ip, int port);
};