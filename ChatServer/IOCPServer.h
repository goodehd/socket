#pragma once
#include <unordered_map>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

class IOCPserver{

public:
	IOCPserver();
	~IOCPserver();

private:
	SOCKET m_listenSocket;

public:
	bool SocketInit();
	bool SocketBind(int port);
	bool SocketListen(int backLog);
};