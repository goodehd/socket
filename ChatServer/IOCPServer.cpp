#include <iostream>
#include <WinSock2.h>
#include <stdexcept>
#include <thread>

#include "IOCPServer.h"
#include "Socket.h"

IOCPserver::IOCPserver():m_hIocp(NULL), m_listenSocket(), m_isRuning(false) { }
IOCPserver::~IOCPserver()  {
	CloseHandle(m_hIocp);
}

bool IOCPserver::Init() {
	if (!InitWSA()) 
		return false;
	if (!InitListenSocket(5555)) 
		return false;
	if (!InitIOCP()) 
		return false;
	if (!BindListenSocketToIOCP()) 
		return false;

	return true;
}

bool IOCPserver::Run(int workerThreadCount) {
	if (m_isRuning)
		return false;

	if (workerThreadCount <= 0) {
		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		workerThreadCount = sysInfo.dwNumberOfProcessors * 2;
	}

	for (int i = 0; i < workerThreadCount; ++i) {
		std::thread([this]() {
			this->WorkerThreadProc();
			}).detach();
	}

	if (!StartAccept()) {
		std::cerr << "StartAccept failed" << std::endl;
		return false;
	}

	m_isRuning = true;
	std::cout << "IOCP Server is running." << std::endl;
	return false;
}

bool IOCPserver::IocpAdd(Socket& socket, void* userPtr) {
	if(!CreateIoCompletionPort((HANDLE)socket.GetSocket(), m_hIocp, (ULONG_PTR)userPtr, 0))
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

bool IOCPserver::InitIOCP() {
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIocp == NULL) {
		std::cout << "Failed to create IOCP: " << GetLastError() << std::endl;
		return false;
	}
	return true;
}

bool IOCPserver::BindListenSocketToIOCP() {
	return IocpAdd(m_listenSocket, nullptr);
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
	overlapped->clientSocket.SetAcceptContext(m_listenSocket);

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


