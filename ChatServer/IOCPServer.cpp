#include <iostream>
#include <stdexcept>

#include "IOCPServer.h"
#include "IOCPListener.h"

IOCPserver::IOCPserver():m_hIocp(NULL), m_isRunning(false), m_threadCount(0) { }
IOCPserver::~IOCPserver()  {}

bool IOCPserver::Init(int workerThreadCount, unsigned short port) {
	if (!InitWSA()) 
		return false;
	if (!InitIOCP(workerThreadCount))
		return false;

	m_listener = std::make_unique<IOCPListener>(this);
	if (!m_listener.get()->InitListener(port))
		return false;

	return true;
}

bool IOCPserver::Run() {
	if (m_isRunning)
		return false;

	m_isRunning = true;

	for (int i = 0; i < m_threadCount; ++i) {
		m_workerThreads.emplace_back(
			[this]() { this->WorkerThreadProc(); }
		);
	}

	for (int i = 0; i < m_threadCount; ++i) {
		if (!m_listener->StartAccept()) {
			std::cerr << "StartAccept failed at idx " << i << std::endl;
			m_isRunning = false;
			for (auto& t : m_workerThreads) {
				if (t.joinable()) t.join();
			}
			m_workerThreads.clear();
			return false;
		}
	}

	std::cout << "Server is running." << std::endl;
	return true;
}

void IOCPserver::Stop() {
	if (!m_isRunning)
		return;

	m_isRunning = false;

	m_listener->CancelPendingAccept();

	for (size_t i = 0; i < m_workerThreads.size(); ++i) {
		PostQueuedCompletionStatus(m_hIocp,
			0,          
			0,          
			nullptr);   
	}

	for (auto& t : m_workerThreads) {
		if (t.joinable())
			t.join();
	}
	m_workerThreads.clear();

	CloseHandle(m_hIocp);
	m_hIocp = nullptr;

	WSACleanup();
}

bool IOCPserver::IocpAdd(SOCKET socket, void* userPtr) {
	if(!CreateIoCompletionPort((HANDLE)socket, m_hIocp, reinterpret_cast<ULONG_PTR>(userPtr), 0))
		return false;
	return true;
}

bool IOCPserver::InitWSA() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Fail WSA init" << std::endl;
		return false;
	}
	return true;
}

bool IOCPserver::InitIOCP(int workerThreadCount) {
	if (workerThreadCount <= 0) {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);
		m_threadCount = sysInfo.dwNumberOfProcessors * 2;
	} else {
		m_threadCount = workerThreadCount;
	}

	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, m_threadCount);
	if (m_hIocp == NULL) {
		std::cout << "Failed to create IOCP: " << GetLastError() << std::endl;
		return false;
	}
	return true;
}

void IOCPserver::HandleRecv(OverlappedContext* recvCtx, ClientSession* session, DWORD bytesTransferred) {
	if (bytesTransferred == 0) {
		std::cout << "session Disconnect !! : " << session->GetAddress() << std::endl;

		SOCKET s = session->GetSocket();
		session->Disconnect();
		m_clientSessions.erase(s);
		return;
	}

	const char* data = recvCtx->ReceiveBuffer;
	int         len = static_cast<int>(bytesTransferred);

	Broadcast(data, len);

	session->PostRecv();
}

void IOCPserver::Broadcast(const char* data, int len) {
	std::vector<std::shared_ptr<ClientSession>> peers;
	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);
		peers.reserve(m_clientSessions.size());
		for (auto& kv : m_clientSessions) {
			peers.push_back(kv.second);
		}
	}

	for (auto& peer : peers) {
		peer->PostSend(data, len);
	}
}


void IOCPserver::WorkerThreadProc() {
	while (m_isRunning) {
		DWORD bytesTransferred = 0;
		ULONG_PTR completionKey = 0;
		LPOVERLAPPED overlapped = NULL;

		BOOL result = GetQueuedCompletionStatus(
			m_hIocp,
			&bytesTransferred,
			&completionKey,
			&overlapped,
			INFINITE
		);

		if (!result && overlapped) {
			IOContext* ctxBase = reinterpret_cast<IOContext*>(overlapped);
			if (ctxBase->OperationType == EOperationType::ACCEPT) {
				delete reinterpret_cast<AcceptContext*>(overlapped);
				continue;
			}
		}

		if (completionKey == 0 && overlapped == NULL) {
			break;
		}

		if (overlapped == NULL) {
			continue;
		}

		ClientSession* session = reinterpret_cast<ClientSession*>(completionKey);
		IOContext* ctxBase = reinterpret_cast<IOContext*>(overlapped);
		std::shared_ptr<ClientSession> newSession = nullptr;

		switch (ctxBase->OperationType)
		{
		case EOperationType::ACCEPT:
			newSession = m_listener.get()->HandleAccept(static_cast<AcceptContext*>(ctxBase));
			if (newSession != nullptr) {
				AddClientSession(newSession);
			}
			break;
		case EOperationType::RECV:
			HandleRecv(static_cast<OverlappedContext*>(ctxBase), session, bytesTransferred);
			break;
		case EOperationType::SEND:
			break;
		default:
			break;
		}
	}
}


