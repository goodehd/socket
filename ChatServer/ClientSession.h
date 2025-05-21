#pragma once

#include "Socket.h"
#include "Structs.h"

class ClientSession
{
private:
    Socket m_ClientSocket;
    std::unique_ptr<OverlappedContext> m_RecvContext;
    std::unique_ptr<OverlappedContext> m_SendContext;
    std::string m_address;

public:
    ClientSession(Socket&& clientSock)
        : m_ClientSocket(std::move(clientSock))
    {
        m_RecvContext = std::make_unique<OverlappedContext>(EOperationType::RECV);
        m_SendContext = std::make_unique<OverlappedContext>(EOperationType::SEND);
    }

    void SetAddress(std::string ip) {
        m_address = ip;
    }
    
    std::string GetAddress() {
        return m_address;
    }

    SOCKET GetSocket() const {
        return m_ClientSocket.GetSocket();
    }

    OverlappedContext* GetRecvContext() {
        return m_RecvContext.get();
    }

    OverlappedContext* GetSendContext() {
        return m_SendContext.get();
    }

    bool PostRecv();
    bool PostSend(const char* data, int len);
    void Disconnect();
};

