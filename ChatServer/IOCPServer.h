#pragma once

#include <unordered_map>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include "Socket.h"

#pragma comment(lib, "ws2_32.lib")

class IOCPserver{

public:
	IOCPserver();
	~IOCPserver();

private:
	HANDLE m_hIocp;
	Socket m_listenSocket;
	
public:
	bool Init();
	bool IocpAdd(Socket& socket, void* userPtr);

private :
	bool InitWSA();
	bool InitListenSocket(int port);
	bool InitIOCP();
	bool BindListenSocketToIOCP();
};