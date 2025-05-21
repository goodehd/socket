#include "NetworkUtil.h"

bool NetworkUtil::GetPeerSockAddr(SOCKET sock, sockaddr_storage& outStor) {
    int len = sizeof(outStor);
    ZeroMemory(&outStor, len);
    if (getpeername(
        sock,
        reinterpret_cast<sockaddr*>(&outStor),
        &len
    ) == SOCKET_ERROR)
    {
        return false;
    }
    return true;
}

std::string NetworkUtil::GetPeerAddrString(SOCKET sock) {
    sockaddr_storage stor;
    if (!GetPeerSockAddr(sock, stor)) {
        return "<peername error: " + std::to_string(WSAGetLastError()) + ">";
    }
    return AddrToString(reinterpret_cast<sockaddr*>(&stor));
}

std::string NetworkUtil::AddrToString(sockaddr* sa) {
    if (sa == nullptr)
        return "add_storage is null";

    char ipbuf[INET6_ADDRSTRLEN] = { 0 };
    uint16_t port = 0;
    if (sa->sa_family == AF_INET) {
        SOCKADDR_IN* ipv4 = reinterpret_cast<SOCKADDR_IN*>(sa);
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipbuf, INET_ADDRSTRLEN);
        port = ntohs(ipv4->sin_port);
    } else if (sa->sa_family == AF_INET6) {
        SOCKADDR_IN6* ipv6 = reinterpret_cast<SOCKADDR_IN6*>(sa);
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipbuf, INET6_ADDRSTRLEN);
        port = ntohs(ipv6->sin6_port);
    } else {
        return "<unknown>";
    }
    return std::string(ipbuf) + ":" + std::to_string(port);
}