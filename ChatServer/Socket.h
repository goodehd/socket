#pragma once

#include <unordered_map>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <memory>

#include "Structs.h"

#pragma comment(lib, "ws2_32.lib")

enum class ProtocolType {
	TCP,
	UDP
};

class Socket {

private:
	SOCKET m_Socket;
	LPFN_ACCEPTEX m_LpfnAcceptEx;
	std::unique_ptr<OverlappedContext> m_OverlappedStruct;

public :
	Socket();
	~Socket();

public :
	SOCKET GetSocket() const {
		return m_Socket;
	}

public:
	bool SocketInit(ProtocolType type);
	bool SocketBind(int port);
	bool SocketListen(int backLog);
	bool AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer);
	bool SetAcceptContext(Socket& listenSocket);
	bool ReceiveOverlapped();
	void CloseSocket();

private :
	bool LoadAcceptExFunc();
};