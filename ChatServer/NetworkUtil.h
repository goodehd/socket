#pragma once

#include <string>
#include <utility>
#include <WinSock2.h>
#include <Mswsock.h>
#include <WS2tcpip.h> 

#pragma comment(lib, "Mswsock.lib")

class NetworkUtil {
public:
    static std::pair<const sockaddr*, const sockaddr*> ExtractAcceptAddrs(const char* buffer);
    static std::string AddrToString(const sockaddr* sa);
};