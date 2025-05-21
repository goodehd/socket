#include <iostream>
#include <WinSock2.h>
#include <stdexcept>
#include <thread>

#include "ClientSession.h"
#include "IOCPServer.h"

IOCPserver::IOCPserver():m_hIocp(NULL), m_listenSocket(), m_isRuning(false), m_threadCount(0) { }
IOCPserver::~IOCPserver()  {
	CloseHandle(m_hIocp);
}

bool IOCPserver::Init(int workerThreadCount, int port) {
	if (!InitWSA()) 
		return false;
	if (!InitListenSocket(port))
		return false;
	if (!InitIOCP(workerThreadCount))
		return false;
	if (!BindListenSocketToIOCP()) 
		return false;

	m_threadCount = workerThreadCount;
	return true;
}

bool IOCPserver::Run() {
	if (m_isRuning)
		return false;

	if (m_threadCount <= 0) {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		m_threadCount = sysInfo.dwNumberOfProcessors * 2;
	}

	for (int i = 0; i < m_threadCount; ++i) {
		std::thread([this]() {
			this->WorkerThreadProc();
			}).detach();
	}

	if (!StartAccept()) {
		std::cerr << "StartAccept failed" << std::endl;
		return false;
	}

	m_isRuning = true;
	std::cout << "Server is running." << std::endl;
	return true;
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

	//std::pair<const sockaddr*, const sockaddr*> info = NetworkUtil::ExtractAcceptAddrs(overlapped->buffer);
	//std::cout << "신규 클라이언트: " << NetworkUtil::AddrToString(info.second) << std::endl;

	std::shared_ptr<ClientSession> session = std::make_shared<ClientSession>(std::move(overlapped->clientSocket));

	if (!IocpAdd(session.get()->GetSocket(), session.get())) {
		std::cout << "Failed to associate client socket with IOCP" << std::endl;
		delete overlapped;
		return;
	}

	m_clientSessions[session->GetSocket()] = session;

	delete overlapped;
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

		IOContext* context = reinterpret_cast<IOContext*>(overlapped);
		switch (context->OperationType)
		{
		case EOperationType::ACCEPT:
			HandleAccept(static_cast<AcceptContext*>(context));
			break;
		case EOperationType::RECV:
			break;
		case EOperationType::SEND:
			break;
		default:
			break;
		}
	}
}


