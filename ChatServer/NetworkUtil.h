#pragma once

#include <string>
#include <utility>
#include <WinSock2.h>
#include <Mswsock.h>
#include <WS2tcpip.h> 

#pragma comment(lib, "Mswsock.lib")

class NetworkUtil {
public:
    static bool GetPeerSockAddr(SOCKET sock, sockaddr_storage& outStor);
    static std::string GetPeerAddrString(SOCKET sock);
    static std::string AddrToString(sockaddr* sa);
};