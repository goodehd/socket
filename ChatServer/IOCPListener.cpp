#include <iostream>

#include "IOCPServer.h"
#include "IOCPListener.h"

IOCPListener::IOCPListener(IOCPserver* owner)
	:m_owner(owner),
	 m_lpfnAcceptEx(nullptr){}

bool IOCPListener::InitListener(unsigned short port) {
	if (!m_listenSocket.SocketInit(ProtocolType::TCP))
		return false;
	if (!m_listenSocket.SocketBind(port))
		return false;
	if (!m_listenSocket.SocketListen(SOMAXCONN)) {
		std::cout << "Fail Socket listen" << std::endl;
		return false;
	}
	if (!m_owner->IocpAdd(m_listenSocket.GetSocket(), nullptr))
		return false;
	if (!LoadAcceptExFunc())
		return false;

	return true;
}

bool IOCPListener::StartAccept() {
	AcceptContext* clientcontext = new AcceptContext();
	if (!clientcontext->clientSocket.SocketInit(ProtocolType::TCP)) {
		std::cout << "Failed to create client socket for AcceptEx" << std::endl;
		delete clientcontext;
		return false;
	}

	bool immediate;
	if (!AcceptExSocket(clientcontext->clientSocket, &(clientcontext->Overlapped), clientcontext->buffer, immediate)) {
		delete clientcontext;
		return false;
	}

	if (immediate) {
		std::shared_ptr<ClientSession> session = HandleAccept(clientcontext);
		if (session) {
			m_owner->AddClientSession(session);
		}
	}

	return true;
}

void IOCPListener::CancelPendingAccept() {
	CancelIoEx((HANDLE)m_listenSocket.GetSocket(), nullptr);
}

std::shared_ptr<ClientSession> IOCPListener::HandleAccept(AcceptContext* overlapped) {
	if (!SetAcceptContext(overlapped->clientSocket)) {
		std::cout << "SetAcceptContext failed" << std::endl;
		delete overlapped;
		return nullptr;
	}

	StartAccept();

	std::string clientAddr = NetworkUtil::GetPeerAddrString(overlapped->clientSocket.GetSocket());
	std::cout << "신규 클라이언트: " << clientAddr << std::endl;

	std::shared_ptr<ClientSession> session = std::make_shared<ClientSession>(std::move(overlapped->clientSocket));
	session->SetAddress(clientAddr);

	if (!m_owner->IocpAdd(session.get()->GetSocket(), session.get())) {
		std::cout << "Failed to associate client socket with IOCP" << std::endl;
		delete overlapped;
		return nullptr;
	}

	if (!session->PostRecv()) {
		std::cout << "Initial PostRecv failed for: " << clientAddr << std::endl;
		delete overlapped;
		return nullptr;
	}

	delete overlapped;

	return session;
}

bool IOCPListener::SetAcceptContext(Socket& clientSock) {
	SOCKET listenSock = m_listenSocket.GetSocket();
	if (setsockopt(
		clientSock.GetSocket(),
		SOL_SOCKET,
		SO_UPDATE_ACCEPT_CONTEXT,
		reinterpret_cast<const char*>(&listenSock),
		sizeof(listenSock)
	) == SOCKET_ERROR)
	{
		std::cout << "SO_UPDATE_ACCEPT_CONTEXT failed: "
			<< WSAGetLastError() << std::endl;
		return false;
	}
	return true;
}

bool IOCPListener::AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer, bool& outImmediate) {
	DWORD bytesReceived = 0;

	BOOL result = m_lpfnAcceptEx(
		m_listenSocket.GetSocket(),
		clientSocket.GetSocket(),
		buffer,
		0,
		sizeof(SOCKADDR_STORAGE) + 16,
		sizeof(SOCKADDR_STORAGE) + 16,
		&bytesReceived,
		overlapped
	);

	if (result) {
		outImmediate = true;
		return true;
	}

	int err = WSAGetLastError();
	if (err == ERROR_IO_PENDING) {
		outImmediate = false;
		return true;
	}

	std::cout << "AcceptEx failed with error: " << err << std::endl;
	return false;
}

bool IOCPListener::LoadAcceptExFunc() {
	GUID guidAcceptEx = WSAID_ACCEPTEX;
	DWORD bytes = 0;

	return WSAIoctl(
		m_listenSocket.GetSocket(),
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidAcceptEx,
		sizeof(guidAcceptEx),
		&m_lpfnAcceptEx,
		sizeof(m_lpfnAcceptEx),
		&bytes,
		NULL,
		NULL) != SOCKET_ERROR;
}