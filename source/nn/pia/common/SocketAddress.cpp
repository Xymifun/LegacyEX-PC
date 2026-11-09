#include "nn/pia/common/SocketAddress.hpp"

#include <coreinit/memory.h>

#include "nn/pia/String.hpp"

extern "C"
{
int getaddrinfo(const char* host, const char* service, const nn::pia::common::AddrInfo* hints,
                nn::pia::common::AddrInfo** result);
void freeaddrinfo(nn::pia::common::AddrInfo* info);
const char* gai_strerror(int error);
}

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
enum
{
    cFamilyInet = 2,
    cFormattedLength = 32
};
}

void SocketAddress::Init()
{
    m_pointer = &m_address;
    OSBlockSet(&m_address, 0, sizeof(m_address));
    m_pointer->family = cFamilyInet;
}

void SocketAddress::SetInetAddress(const InetAddress& address)
{
    m_pointer->address = address.m_address;
    m_pointer->port = address.m_port;
}

void SocketAddress::GetInetAddress(InetAddress* address)
{
    if (address == 0)
        return;
    address->m_address = m_pointer->address;
    address->m_port = m_pointer->port;
}

void SocketAddress::SetSockAddrIn(const SockAddrIn& address)
{
    *m_pointer = address;
}

SockAddrIn SocketAddress::GetSockAddrIn()
{
    return *m_pointer;
}

bool SocketAddress::GetAddressInfo(const char* host, const char* service, const AddrInfo* hints)
{
    bool success = false;
    AddrInfo* result;
    int error;
    if (hints != 0)
    {
        error = getaddrinfo(host, service, hints, &result);
    }
    else
    {
        AddrInfo defaultHints;
        OSBlockSet(&defaultHints, 0, sizeof(defaultHints));
        defaultHints.family = cFamilyInet;
        error = getaddrinfo(host, service, &defaultHints, &result);
    }
    if (error == 0)
    {
        m_pointer->address = result->address->address;
        freeaddrinfo(result);
        success = true;
    }
    else
    {
        gai_strerror(error);
    }
    return success;
}

void SocketAddress::Trace(unsigned long long value) const
{
    FixedString<cFormattedLength> text;
    const unsigned int address = m_pointer->address;
    text.Format("%d.%d.%d.%d:%d", address >> 24, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff,
                m_pointer->port);
}

const SockAddrIn* SocketAddress::GetSockAddrInPointer() const
{
    return m_pointer;
}

}
}
}
