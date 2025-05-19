#pragma once

#include <unordered_map>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

constexpr int MaxReceiveLength = 1024;

enum class ProtocolType {
	TCP,
	UDP
};

struct OverlappedContext {
	WSAOVERLAPPED ReadOverlappedStruct; // recv용 구조체
	WSABUF WsaBuf;
	DWORD LappedFlag; // recvFlag
	int OperationType;
	char ReceiveBuffer[MaxReceiveLength];

	OverlappedContext() {
		ZeroMemory(&ReadOverlappedStruct, sizeof(WSAOVERLAPPED));
		WsaBuf.buf = ReceiveBuffer;
		WsaBuf.len = MaxReceiveLength;
		ZeroMemory(ReceiveBuffer, MaxReceiveLength);
		OperationType = 0;
		LappedFlag = 0;
	}
};

class Socket {

private:
	SOCKET m_Socket;
	LPFN_ACCEPTEX m_LpfnAcceptEx;
	OverlappedContext m_OverlappedStruct;

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
	bool AcceptExSocket(Socket clientSocket, OVERLAPPED* overlapped, char* buffer, DWORD bufferLen);
	bool SetAcceptContext(SOCKET listenSocket);
	bool ReceiveOverlapped();
	void CloseSocket();

private :
	bool LoadAcceptExFunc();
};