#include <iostream>
#include "ClientSession.h"

bool ClientSession::PostRecv() {
    OverlappedContext* ctx = m_RecvContext.get();

    ZeroMemory(&ctx->Overlapped, sizeof(OVERLAPPED));
    ctx->OperationType = EOperationType::RECV;

    ZeroMemory(ctx->ReceiveBuffer, MaxReceiveLength);

    DWORD flags = 0;
    int result = WSARecv(
        m_ClientSocket.GetSocket(),  
        &ctx->WsaBuf,                
        1,                           
        nullptr,                     
        &flags,                      
        &ctx->Overlapped,            
        nullptr                      
    );

    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            std::cerr << "WSARecv failed: " << err << std::endl;
            return false;
        }
    }
    return true;
}

bool ClientSession::PostSend(const char* data, int len) {
    OverlappedContext* ctx = m_SendContext.get();

    ZeroMemory(&ctx->Overlapped, sizeof(OVERLAPPED));
    ctx->OperationType = EOperationType::SEND;

    ctx->WsaBuf.buf = const_cast<char*>(data);
    ctx->WsaBuf.len = len;

    int result = WSASend(
        m_ClientSocket.GetSocket(),
        &ctx->WsaBuf,   
        1,              
        nullptr,        
        0,              
        &ctx->Overlapped,
        nullptr        
    );

    if (result == SOCKET_ERROR) {
        int err = WSAGetLastError();
        if (err != WSA_IO_PENDING) {
            std::cerr << "WSASend failed: " << err << std::endl;
            return false;
        }
    }

    return true;
}

void ClientSession::Disconnect() {
    m_ClientSocket.CloseSocket();
}