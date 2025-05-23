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
#include "ClientSession.h"

#pragma comment(lib, "ws2_32.lib")

class IOCPListener;

class IOCPserver{

public:
	IOCPserver();
	~IOCPserver();

private:
	HANDLE m_hIocp;
	std::unique_ptr<IOCPListener> m_listener;
	std::unordered_map<SOCKET, std::shared_ptr<ClientSession>> m_clientSessions;
	std::vector<std::thread> m_workerThreads;
	std::mutex m_sessionsMutex;

	bool m_isRunning;
	int m_threadCount;
	
public:
	void AddClientSession(std::shared_ptr<ClientSession> session) {
		{
			std::lock_guard<std::mutex> lock(m_sessionsMutex);
			m_clientSessions[session.get()->GetSocket()] = session;
		}
	}

public:
	bool Init(int workerThreadCount, unsigned short port);
	bool Run();
	void Stop();
	bool IocpAdd(SOCKET socket, void* userPtr);
	void Broadcast(const char* data, int len);

private :
	bool InitWSA();
	bool InitIOCP(int workerThreadCount);
	void WorkerThreadProc();

	void HandleRecv(OverlappedContext* overlapped, ClientSession* session, DWORD bytesTransferred);
};