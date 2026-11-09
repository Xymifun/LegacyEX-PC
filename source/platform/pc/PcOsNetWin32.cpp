#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#include "platform/pc/PcOs.hpp"

#include <stdio.h>
#include <string.h>

namespace
{
const int LISTEN_BACKLOG = 8;

SOCKET ToNative(PcOsSocket sock)
{
    return static_cast<SOCKET>(sock);
}

PcOsSocket FromNative(SOCKET sock)
{
    return sock == INVALID_SOCKET ? kPcOsInvalidSocket : static_cast<PcOsSocket>(sock);
}

sockaddr_in MakeAddress(unsigned int addr, unsigned short port)
{
    sockaddr_in sa;
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = htonl(addr);
    sa.sin_port = htons(port);
    return sa;
}

void EnableOption(SOCKET sock, int level, int option)
{
    BOOL on = TRUE;
    setsockopt(sock, level, option, reinterpret_cast<const char*>(&on), sizeof(on));
}
}

bool PcOsNetInitialise()
{
    static bool s_initialised = false;
    if (s_initialised)
        return true;
    WSADATA data;
    s_initialised = WSAStartup(MAKEWORD(2, 2), &data) == 0;
    return s_initialised;
}

size_t PcOsListIPv4Broadcasts(unsigned int* out, size_t cap)
{
    SOCKET probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (probe == INVALID_SOCKET)
        return 0;
    INTERFACE_INFO ifs[32];
    DWORD bytes = 0;
    size_t n = 0;
    if (WSAIoctl(probe, SIO_GET_INTERFACE_LIST, 0, 0, ifs, sizeof(ifs), &bytes, 0, 0) == 0)
    {
        size_t numIfs = bytes / sizeof(INTERFACE_INFO);
        for (size_t i = 0; i < numIfs && n < cap; ++i)
        {
            const INTERFACE_INFO& info = ifs[i];
            if ((info.iiFlags & IFF_UP) == 0 || (info.iiFlags & IFF_LOOPBACK) != 0
                || (info.iiFlags & IFF_BROADCAST) == 0)
                continue;
            unsigned int addr = ntohl(info.iiAddress.AddressIn.sin_addr.s_addr);
            unsigned int mask = ntohl(info.iiNetmask.AddressIn.sin_addr.s_addr);
            out[n++] = addr | ~mask;
        }
    }
    closesocket(probe);
    return n;
}

size_t PcOsListIPv4Addresses(unsigned int* out, size_t cap)
{
    SOCKET probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (probe == INVALID_SOCKET)
        return 0;
    INTERFACE_INFO ifs[32];
    DWORD bytes = 0;
    size_t n = 0;
    if (WSAIoctl(probe, SIO_GET_INTERFACE_LIST, 0, 0, ifs, sizeof(ifs), &bytes, 0, 0) == 0)
    {
        size_t numIfs = bytes / sizeof(INTERFACE_INFO);
        for (size_t i = 0; i < numIfs && n < cap; ++i)
        {
            const INTERFACE_INFO& info = ifs[i];
            if ((info.iiFlags & IFF_UP) == 0 || (info.iiFlags & IFF_LOOPBACK) != 0)
                continue;
            out[n++] = ntohl(info.iiAddress.AddressIn.sin_addr.s_addr);
        }
    }
    closesocket(probe);
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
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
        return kPcOsInvalidSocket;
    EnableOption(s, SOL_SOCKET, SO_EXCLUSIVEADDRUSE);
    sockaddr_in sa = MakeAddress(INADDR_ANY, port);
    if (bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) != 0
        || listen(s, LISTEN_BACKLOG) != 0)
    {
        closesocket(s);
        return kPcOsInvalidSocket;
    }
    return FromNative(s);
}

PcOsSocket PcOsTcpAccept(PcOsSocket listener, unsigned int* addr)
{
    sockaddr_in peer;
    int len = sizeof(peer);
    SOCKET s = accept(ToNative(listener), reinterpret_cast<sockaddr*>(&peer), &len);
    if (s == INVALID_SOCKET)
        return kPcOsInvalidSocket;
    EnableOption(s, IPPROTO_TCP, TCP_NODELAY);
    if (addr != 0)
        *addr = ntohl(peer.sin_addr.s_addr);
    return FromNative(s);
}

PcOsSocket PcOsTcpConnect(unsigned int addr, unsigned short port)
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
        return kPcOsInvalidSocket;
    sockaddr_in sa = MakeAddress(addr, port);
    if (connect(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) != 0)
    {
        closesocket(s);
        return kPcOsInvalidSocket;
    }
    EnableOption(s, IPPROTO_TCP, TCP_NODELAY);
    return FromNative(s);
}

bool PcOsSocketSendAll(PcOsSocket sock, const void* data, size_t len)
{
    const char* p = static_cast<const char*>(data);
    while (len > 0)
    {
        int n = send(ToNative(sock), p, (int)len, 0);
        if (n <= 0)
            return false;
        p += n;
        len -= (size_t)n;
    }
    return true;
}

int PcOsSocketReceive(PcOsSocket sock, void* buf, size_t len)
{
    int n = recv(ToNative(sock), static_cast<char*>(buf), (int)len, 0);
    return n < 0 ? -1 : n;
}

int PcOsSocketPeek(PcOsSocket sock, void* buf, size_t len)
{
    int n = recv(ToNative(sock), static_cast<char*>(buf), (int)len, MSG_PEEK);
    return n < 0 ? -1 : n;
}

int PcOsSocketWaitReadable(PcOsSocket sock, unsigned int ms)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(ToNative(sock), &fds);
    timeval tv;
    tv.tv_sec = (long)(ms / 1000);
    tv.tv_usec = (long)(ms % 1000 * 1000);
    int n = select(0, &fds, 0, 0, &tv);
    return n < 0 ? -1 : n;
}

void PcOsSocketSetSendTimeout(PcOsSocket sock, unsigned int ms)
{
    DWORD v = ms;
    setsockopt(ToNative(sock), SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&v), sizeof(v));
}

void PcOsSocketShutdown(PcOsSocket sock)
{
    shutdown(ToNative(sock), SD_BOTH);
}

void PcOsSocketClose(PcOsSocket sock)
{
    closesocket(ToNative(sock));
}

PcOsSocket PcOsUdpOpen(unsigned short port)
{
    SOCKET s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s == INVALID_SOCKET)
        return kPcOsInvalidSocket;
    EnableOption(s, SOL_SOCKET, SO_REUSEADDR);
    EnableOption(s, SOL_SOCKET, SO_BROADCAST);
    sockaddr_in sa = MakeAddress(INADDR_ANY, port);
    if (bind(s, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) != 0)
    {
        closesocket(s);
        return kPcOsInvalidSocket;
    }
    return FromNative(s);
}

bool PcOsUdpSendTo(PcOsSocket sock, unsigned int addr, unsigned short port, const void* data, size_t len)
{
    sockaddr_in sa = MakeAddress(addr, port);
    return sendto(ToNative(sock), static_cast<const char*>(data), (int)len, 0,
                  reinterpret_cast<sockaddr*>(&sa), sizeof(sa))
           == (int)len;
}

int PcOsUdpReceiveFrom(PcOsSocket sock, void* buf, size_t len, unsigned int* addr,
                       unsigned int ms)
{
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(ToNative(sock), &fds);
    timeval tv;
    tv.tv_sec = (long)(ms / 1000);
    tv.tv_usec = (long)(ms % 1000 * 1000);
    if (select(0, &fds, 0, 0, &tv) <= 0)
        return -1;
    sockaddr_in peer;
    int salen = sizeof(peer);
    int n = recvfrom(ToNative(sock), static_cast<char*>(buf), (int)len, 0,
                            reinterpret_cast<sockaddr*>(&peer), &salen);
    if (n < 0)
        return -1;
    if (addr != 0)
        *addr = ntohl(peer.sin_addr.s_addr);
    return n;
}

#endif
