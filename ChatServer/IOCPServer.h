#pragma once

#include <unordered_map>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include "Socket.h"

#pragma comment(lib, "ws2_32.lib")

struct AcceptContext {
	OVERLAPPED overlapped;
	char buffer[2 * (sizeof(SOCKADDR_IN) + 16)];
	Socket clientSocket;
};

class IOCPserver{

public:
	IOCPserver();
	~IOCPserver();

private:
	HANDLE m_hIocp;
	Socket m_listenSocket;
	std::unordered_map<Socket*, AcceptContext> m_clientMap;
	
public:
	bool Init();
	bool Run(int workerThreadCount = 0);
	void Stop();
	bool IocpAdd(Socket& socket, void* userPtr);

private :
	bool InitWSA();
	bool InitListenSocket(int port);
	bool InitIOCP();
	bool BindListenSocketToIOCP();
	bool StartAccept();
	void HandleAccept(AcceptContext* overlapped);
};