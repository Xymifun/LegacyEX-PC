#ifndef _WIN32

#include "platform/pc/PcOs.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace
{
const int LISTEN_BACKLOG = 8;

sockaddr_in MakeAddress(unsigned int addr, unsigned short port)
{
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    return sa;
}

void EnableOption(int sock, int level, int option)
{
    int on = 1;
    setsockopt(sock, level, option, &on, sizeof(on));
}
}

bool PcOsNetInitialise()
{
    return true;
}

size_t PcOsListIPv4Broadcasts(unsigned int* out, size_t cap)
{
    ifaddrs* ifs = 0;
    if (getifaddrs(&ifs) != 0)
        return 0;
    size_t n = 0;
    for (ifaddrs* e = ifs; e != 0 && n < cap; e = e->ifa_next)
    {
        if (e->ifa_addr == 0 || e->ifa_netmask == 0 || e->ifa_addr->sa_family != AF_INET
            || (e->ifa_flags & IFF_UP) == 0 || (e->ifa_flags & IFF_LOOPBACK) != 0
            || (e->ifa_flags & IFF_BROADCAST) == 0)
            continue;
        unsigned int addr = ntohl(reinterpret_cast<sockaddr_in*>(e->ifa_addr)->sin_addr.s_addr);
        unsigned int mask = ntohl(reinterpret_cast<sockaddr_in*>(e->ifa_netmask)->sin_addr.s_addr);
        out[n++] = addr | ~mask;
    }
    freeifaddrs(ifs);
    return n;
}

size_t PcOsListIPv4Addresses(unsigned int* out, size_t cap)
{
    ifaddrs* ifs = 0;
    if (getifaddrs(&ifs) != 0)
        return 0;
    size_t n = 0;
    for (ifaddrs* e = ifs; e != 0 && n < cap; e = e->ifa_next)
    {
        if (e->ifa_addr == 0 || e->ifa_addr->sa_family != AF_INET || (e->ifa_flags & IFF_UP) == 0
            || (e->ifa_flags & IFF_LOOPBACK) != 0)
            continue;
        out[n++] = ntohl(reinterpret_cast<sockaddr_in*>(e->ifa_addr)->sin_addr.s_addr);
    }
    freeifaddrs(ifs);
    return n;
}

bool PcOsParseIPv4(const char* s, unsigned int* out)
{
    in_addr a;
    if (inet_pton(AF_INET, s, &a) != 1)
        return false;
    *out = ntohl(a.s_addr);
    return true;
}

bool PcOsResolveIPv4(const char* host, unsigned int* out)
{
    if (PcOsParseIPv4(host, out))
        return true;
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    addrinfo* res = 0;
    if (getaddrinfo(host, 0, &hints, &res) != 0 || res == 0)
        return false;
    *out = ntohl(reinterpret_cast<sockaddr_in*>(res->ai_addr)->sin_addr.s_addr);
    freeaddrinfo(res);
    return true;
}

void PcOsFormatIPv4(unsigned int addr, char* buf, size_t len)
{
    snprintf(buf, len, "%u.%u.%u.%u", addr >> 24, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF,
             addr & 0xFF);
}

PcOsSocket PcOsTcpListen(unsigned short port)
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
        return kPcOsInvalidSocket;
    EnableOption(s, SOL_SOCKET, SO_REUSEADDR);
    sockaddr_in sa = MakeAddress(INADDR_ANY, port);
    if (bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) != 0
        || listen(s, LISTEN_BACKLOG) != 0)
    {
        close(s);
        return kPcOsInvalidSocket;
    }
    return s;
}

PcOsSocket PcOsTcpAccept(PcOsSocket listener, unsigned int* addr)
{
    sockaddr_in peer;
    socklen_t len = sizeof(peer);
    int s = accept(static_cast<int>(listener), reinterpret_cast<sockaddr*>(&peer), &len);
    if (s < 0)
        return kPcOsInvalidSocket;
    EnableOption(s, IPPROTO_TCP, TCP_NODELAY);
    if (addr != 0)
        *addr = ntohl(peer.sin_addr.s_addr);
    return s;
}

PcOsSocket PcOsTcpConnect(unsigned int addr, unsigned short port)
{
    int s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s < 0)
        return kPcOsInvalidSocket;
    sockaddr_in sa = MakeAddress(addr, port);
    if (connect(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) != 0)
    {
        close(s);
        return kPcOsInvalidSocket;
    }
    EnableOption(s, IPPROTO_TCP, TCP_NODELAY);
    return s;
}

bool PcOsSocketSendAll(PcOsSocket sock, const void* data, size_t len)
{
    const char* p = static_cast<const char*>(data);
    while (len > 0)
    {
        ssize_t n = send(static_cast<int>(sock), p, len, MSG_NOSIGNAL);
        if (n <= 0)
            return false;
        p += n;
        len -= static_cast<size_t>(n);
    }
    return true;
}

int PcOsSocketReceive(PcOsSocket sock, void* buf, size_t len)
{
    ssize_t n = recv(static_cast<int>(sock), buf, len, 0);
    return n < 0 ? -1 : static_cast<int>(n);
}

int PcOsSocketPeek(PcOsSocket sock, void* buf, size_t len)
{
    ssize_t n = recv(static_cast<int>(sock), buf, len, MSG_PEEK);
    return n < 0 ? -1 : static_cast<int>(n);
}

int PcOsSocketWaitReadable(PcOsSocket sock, unsigned int ms)
{
    pollfd pfd;
    pfd.fd = static_cast<int>(sock);
    pfd.events = POLLIN;
    pfd.revents = 0;
    int n = poll(&pfd, 1, static_cast<int>(ms));
    return n < 0 ? -1 : n;
}

void PcOsSocketSetSendTimeout(PcOsSocket sock, unsigned int ms)
{
    timeval tv;
    tv.tv_sec = static_cast<long>(ms / 1000);
    tv.tv_usec = static_cast<long>(ms % 1000 * 1000);
    setsockopt(static_cast<int>(sock), SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

void PcOsSocketShutdown(PcOsSocket sock)
{
    shutdown(static_cast<int>(sock), SHUT_RDWR);
}

void PcOsSocketClose(PcOsSocket sock)
{
    close(static_cast<int>(sock));
}

PcOsSocket PcOsUdpOpen(unsigned short port)
{
    int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
        return kPcOsInvalidSocket;
    EnableOption(s, SOL_SOCKET, SO_REUSEADDR);
    EnableOption(s, SOL_SOCKET, SO_BROADCAST);
    sockaddr_in sa = MakeAddress(INADDR_ANY, port);
    if (bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) != 0)
    {
        close(s);
        return kPcOsInvalidSocket;
    }
    return s;
}

bool PcOsUdpSendTo(PcOsSocket sock, unsigned int addr, unsigned short port, const void* data, size_t len)
{
    sockaddr_in sa = MakeAddress(addr, port);
    return sendto(static_cast<int>(sock), data, len, 0, reinterpret_cast<sockaddr*>(&sa), sizeof(sa))
           == static_cast<ssize_t>(len);
}

int PcOsUdpReceiveFrom(PcOsSocket sock, void* buf, size_t len, unsigned int* addr,
                       unsigned int ms)
{
    pollfd fds;
    fds.fd = static_cast<int>(sock);
    fds.events = POLLIN;
    fds.revents = 0;
    if (poll(&fds, 1, static_cast<int>(ms)) <= 0)
        return -1;
    sockaddr_in peer;
    socklen_t salen = sizeof(peer);
    ssize_t n = recvfrom(static_cast<int>(sock), buf, len, 0, reinterpret_cast<sockaddr*>(&peer),
                                &salen);
    if (n < 0)
        return -1;
    if (addr != 0)
        *addr = ntohl(peer.sin_addr.s_addr);
    return static_cast<int>(n);
}

#endif
