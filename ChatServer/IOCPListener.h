#pragma once

#include <windows.h>
#include <mswsock.h>
#include <functional>
#include <memory>

#include "Socket.h"
#include "Structs.h"

class IOCPserver;
class ClientSession;

class IOCPListener {
private:
    IOCPserver* m_owner;
    Socket m_listenSocket;
    LPFN_ACCEPTEX m_lpfnAcceptEx;

public:
    IOCPListener(IOCPserver* owner);
    bool InitListener(unsigned short port);
    bool StartAccept();
    void CancelPendingAccept();
    std::shared_ptr<ClientSession> HandleAccept(AcceptContext* overlapped);

private:
    bool LoadAcceptExFunc();
    bool AcceptExSocket(Socket& clientSocket, OVERLAPPED* overlapped, char* buffer, bool& outImmediate);
    bool SetAcceptContext(Socket& clientSock);
};