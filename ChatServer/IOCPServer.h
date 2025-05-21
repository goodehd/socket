#pragma once

#include <unordered_map>
#include <memory>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <thread>
#include <vector>
#include <mutex>

#include "Socket.h"
#include "Structs.h"
#include "NetworkUtil.h"

class ClientSession;

#pragma comment(lib, "ws2_32.lib")

struct AcceptContext : IOContext {
	char buffer[2 * (sizeof(SOCKADDR_STORAGE) + 16)];
	Socket clientSocket;

	AcceptContext() {
		OperationType = EOperationType::ACCEPT;
		ZeroMemory(buffer, sizeof(buffer));
	}
};

class IOCPserver{

public:
	IOCPserver();
	~IOCPserver();

private:
	HANDLE m_hIocp;
	Socket m_listenSocket;
	std::unordered_map<SOCKET, std::shared_ptr<ClientSession>> m_clientSessions;
	std::vector<std::thread> m_workerThreads;
	std::mutex m_sessionsMutex;

	bool m_isRuning;
	int m_threadCount;
	
public:
	bool Init(int workerThreadCount, int port);
	bool Run();
	void Stop();
	bool IocpAdd(SOCKET socket, void* userPtr);
	void Broadcast(const char* data, int len);

private :
	bool InitWSA();
	bool InitListenSocket(int port);
	bool InitIOCP(int workerThreadCount);
	bool BindListenSocketToIOCP();
	bool StartAccept();
	void WorkerThreadProc();

	void HandleAccept(AcceptContext* overlapped);
	void HandleRecv(OverlappedContext* overlapped, ClientSession* session, DWORD bytesTransferred);

};