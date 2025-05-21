#pragma once

#include <windows.h>
#include <mswsock.h>
#include <functional>

#include "Socket.h"
#include "IOCPServer.h"

using AcceptHandler = std::function<void(AcceptContext*)>;

class IOCPListener {
private:
    IOCPserver&          m_owner;
    Socket               m_listenSocket;
    LPFN_ACCEPTEX        m_lpfnAcceptEx;
    AcceptHandler        m_handler;
    unsigned short       m_port;

public:
    IOCPListener(unsigned short port, IOCPserver owner, AcceptHandler handler);
    bool InitListener();
    void InitAccept(int initialCount);
    void OnAccept(AcceptContext* ctx);

private:
    void PostAccept();
    bool LoadAcceptExFunc();
    bool AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer)
};