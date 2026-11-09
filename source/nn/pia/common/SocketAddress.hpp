#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_ADDRESS_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_ADDRESS_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/InetAddress.hpp"

namespace nn
{
namespace pia
{
namespace common
{

struct SockAddrIn
{
    unsigned short family;
    unsigned short port;
    unsigned int address;
    unsigned char zero[8];
};

struct AddrInfo
{
    int flags;
    int family;
    int socketType;
    int protocol;
    unsigned int addressLength;
    char* canonicalName;
    SockAddrIn* address;
    AddrInfo* next;
};

class SocketAddress
{
public:
    SocketAddress() { Init(); }

    void Init();
    void SetInetAddress(const InetAddress& address);
    void GetInetAddress(InetAddress* address);
    void SetSockAddrIn(const SockAddrIn& address);
    SockAddrIn GetSockAddrIn();
    bool GetAddressInfo(const char* host, const char* service, const AddrInfo* hints);
    void Trace(unsigned long long value) const;
    const SockAddrIn* GetSockAddrInPointer() const;
    void GetHostByName(const char* host);

private:
    SockAddrIn* m_pointer;
    SockAddrIn m_address;
    unsigned int m_unknownTail[4];
};

}
}
}

#endif
