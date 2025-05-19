#include "IOCPServer.h"
#include "Socket.h"
#include <iostream>
#include <WinSock2.h>
#include <stdexcept>

IOCPserver::IOCPserver():m_hIocp(NULL), m_listenSocket() { }
IOCPserver::~IOCPserver() 
{
	CloseHandle(m_hIocp);
}

bool IOCPserver::Init()
{
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

bool IOCPserver::IocpAdd(Socket& socket, void* userPtr)
{
	if(!CreateIoCompletionPort((HANDLE)socket.GetSocket(), m_hIocp, (ULONG_PTR)userPtr, 0))
		return false;
	return true;
}

bool IOCPserver::InitWSA()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		std::cout << "Fail WSA init" << std::endl;
		return false;
	}
}

bool IOCPserver::InitListenSocket(int port)
{
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

bool IOCPserver::InitIOCP()
{
	m_hIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hIocp == NULL) {
		std::cout << "Failed to create IOCP: " << GetLastError() << std::endl;
		return false;
	}
	return true;
}

bool IOCPserver::BindListenSocketToIOCP()
{
	return IocpAdd(m_listenSocket, nullptr);
}

bool IOCPserver::StartAccept()
{
	AcceptContext* clientcontext = new AcceptContext();
	if (!clientcontext->clientSocket.SocketInit(ProtocolType::TCP)) {
		std::cout << "Failed to create client socket for AcceptEx" << std::endl;
		delete clientcontext;
		return false;
	}

	m_listenSocket.AcceptExSocket(clientcontext->clientSocket, &(clientcontext->overlapped), clientcontext->buffer);

	return false;
}


