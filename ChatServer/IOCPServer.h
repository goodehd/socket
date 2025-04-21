#pragma once
#include <unordered_map>


class IOCPserver{

public:
	IOCPserver();
	~IOCPserver();

private:
	SOCKET m_listenSocket;

public:
	bool SocketInit();
	void SocketBind();
	void SocketListen();

};