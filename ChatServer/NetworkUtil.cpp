#include "NetworkUtil.h"

std::pair<const sockaddr*, const sockaddr*> NetworkUtil::ExtractAcceptAddrs(const char* buffer) {
    const sockaddr* localSa = nullptr;
    const sockaddr* remoteSa = nullptr;
    int localLen = 0;
    int remoteLen = 0;

    ::GetAcceptExSockaddrs(
        (void*)buffer,
        0,
        sizeof(SOCKADDR_IN) + 16,
        sizeof(SOCKADDR_IN) + 16,
        (sockaddr**)&localSa,
        &localLen,
        (sockaddr**)&remoteSa,
        &remoteLen
    );

    return { localSa, remoteSa };
}

std::string NetworkUtil::AddrToString(const sockaddr* sa) {
    char ipbuf[INET6_ADDRSTRLEN] = { 0 };
    uint16_t port = 0;
    if (sa->sa_family == AF_INET) {
        auto* sin = (sockaddr_in*)sa;
        InetNtopA(AF_INET, &sin->sin_addr, ipbuf, sizeof(ipbuf));
        port = ntohs(sin->sin_port);
    } else if (sa->sa_family == AF_INET6) {
        auto* sin6 = (sockaddr_in6*)sa;
        InetNtopA(AF_INET6, &sin6->sin6_addr, ipbuf, sizeof(ipbuf));
        port = ntohs(sin6->sin6_port);
    } else {
        return "<unknown>";
    }
    return std::string(ipbuf) + ":" + std::to_string(port);
}