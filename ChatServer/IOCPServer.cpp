#include <iostream>
#include <WinSock2.h>
#include <stdexcept>

#include "ClientSession.h"
#include "IOCPServer.h"

IOCPserver::IOCPserver():m_hIocp(NULL), m_listenSocket(), m_isRuning(false), m_threadCount(0) { }
IOCPserver::~IOCPserver()  {}

bool IOCPserver::Init(int workerThreadCount, int port) {
	if (!InitWSA()) 
		return false;
	if (!InitListenSocket(port)) // 옮김
		return false;
	if (!InitIOCP(workerThreadCount))
		return false;
	if (!BindListenSocketToIOCP()) //옮김
		return false;

	return true;
}

bool IOCPserver::Run() {
	if (m_isRuning)
		return false;

	for (int i = 0; i < m_threadCount; ++i) {
		m_workerThreads.emplace_back(
			[this]() { this->WorkerThreadProc(); }
		);
	}

	if (!StartAccept()) {
		std::cout << "StartAccept failed" << std::endl;
		return false;
	}

	m_isRuning = true;
	std::cout << "Server is running." << std::endl;
	return true;
}

void IOCPserver::Stop() {
	if (!m_isRuning)
		return;

	m_isRuning = false;

	m_listenSocket.CloseSocket();

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

bool IOCPserver::InitListenSocket(int port) {
	if (!m_listenSocket.SocketInit(ProtocolType::TCP)) {
		return false;
	}

	if (!m_listenSocket.SocketBind(port)) {
		return false;
	}

	if (!m_listenSocket.SocketListen(SOMAXCONN)) {
		std::cout << "Fail Socket listen" << std::endl;
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

	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, workerThreadCount);
	if (m_hIocp == NULL) {
		std::cout << "Failed to create IOCP: " << GetLastError() << std::endl;
		return false;
	}
	return true;
}

bool IOCPserver::BindListenSocketToIOCP() {
	return IocpAdd(m_listenSocket.GetSocket(), nullptr);
}

bool IOCPserver::StartAccept() {
	AcceptContext* clientcontext = new AcceptContext();
	if (!clientcontext->clientSocket.SocketInit(ProtocolType::TCP)) {
		std::cout << "Failed to create client socket for AcceptEx" << std::endl;
		delete clientcontext;
		return false;
	}

	if (!m_listenSocket.AcceptExSocket(clientcontext->clientSocket, &(clientcontext->Overlapped), clientcontext->buffer)) {
		delete clientcontext;
		return false;
	}

	return true;
}

void IOCPserver::HandleAccept(AcceptContext* overlapped) {
	if (!overlapped->clientSocket.SetAcceptContext(m_listenSocket)) {
		std::cout << "SetAcceptContext failed" << std::endl;
		delete overlapped;
		return;
	}

	StartAccept();

	std::string clientAddr = NetworkUtil::GetPeerAddrString(overlapped->clientSocket.GetSocket());
	std::cout << "신규 클라이언트: " << clientAddr << std::endl;

	std::shared_ptr<ClientSession> session = std::make_shared<ClientSession>(std::move(overlapped->clientSocket));
	session->SetAddress(clientAddr);

	if (!IocpAdd(session.get()->GetSocket(), session.get())) {
		std::cout << "Failed to associate client socket with IOCP" << std::endl;
		delete overlapped;
		return;
	}

	{
		std::lock_guard<std::mutex> lock(m_sessionsMutex);
		m_clientSessions[session->GetSocket()] = session;
	}

	if (!session->PostRecv()) {
		std::cout << "Initial PostRecv failed for: " << clientAddr << std::endl;
		session->Disconnect();
		{
			std::lock_guard<std::mutex> lock(m_sessionsMutex);
			m_clientSessions.erase(session->GetSocket());
		}
		delete overlapped;
		return;
	}

	delete overlapped;
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
	while (m_isRuning) {
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

		if (completionKey == 0 && overlapped == NULL) {
			break;
		}

		if (overlapped == NULL) {
			continue;
		}

		ClientSession* session = reinterpret_cast<ClientSession*>(completionKey);
		IOContext* ctxBase = reinterpret_cast<IOContext*>(overlapped);

		switch (ctxBase->OperationType)
		{
		case EOperationType::ACCEPT:
			HandleAccept(static_cast<AcceptContext*>(ctxBase));
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


