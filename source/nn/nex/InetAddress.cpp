#include "nn/nex/InetAddress.hpp"

#include <netdb.h>

#include <cstring>
#include <cwchar>

#include "nn/nex/BerkeleySocketDriver.hpp"
#include "nn/nex/StringConversion.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

namespace
{

bool g_autoLookupEnabled = true;

const unsigned int ADDRESS_BUFFER_LENGTH = 128;
const unsigned int PORT_BUFFER_LENGTH = 16;
const unsigned int NARROW_ADDRESS_BUFFER_LENGTH = 256;
const unsigned int LOOKUP_BUFFER_LENGTH = 64;
const unsigned int LOOKUP_STORAGE_LENGTH = 32;
const unsigned int TRACE_BUFFER_LENGTH = 128;
const unsigned int MINIMUM_ADDRESS_BUFFER_LENGTH = 32;
const unsigned int STREAM_BUFFER_LENGTH = 202;

}

InetAddress::InetAddress()
{
    Init();
}

InetAddress::InetAddress(const InetAddress& rAddress)
{
    *this = rAddress;
}

InetAddress::InetAddress(unsigned int uiAddress, unsigned short uiPortNumber)
{
    Init();
    SetAddress(uiAddress);
    SetPortNumber(uiPortNumber);
}

InetAddress::InetAddress(const wchar_t* pAddress, unsigned short uiPortNumber)
{
    Init();
    SetAddress(pAddress);
    SetPortNumber(uiPortNumber);
}

InetAddress::~InetAddress()
{
}

InetAddress& InetAddress::operator=(const InetAddress& rAddress)
{
    std::memcpy(&m_address, &rAddress.m_address, sizeof(m_address));
    return *this;
}

void InetAddress::Init()
{
    std::memset(&m_address, 0, sizeof(m_address));
    m_address.sin_family = AF_INET;
}

unsigned int InetAddress::GetAddress() const
{
    return ntohl(m_address.sin_addr.s_addr);
}

bool InetAddress::GetAddress(wchar_t* pBuffer, unsigned int uiSize) const
{
    if (uiSize < MINIMUM_ADDRESS_BUFFER_LENGTH)
    {
        return false;
    }

    in_addr address = m_address.sin_addr;

    StringConversion::Char8_2T(inet_ntoa(address), pBuffer, uiSize);
    return true;
}

String InetAddress::GetAddressStr() const
{
    wchar_t buffer[ADDRESS_BUFFER_LENGTH];

    GetAddress(buffer, ADDRESS_BUFFER_LENGTH);
    return String(buffer);
}

bool InetAddress::SetAddress(const wchar_t* pAddress)
{
    wchar_t resolvedAddress[LOOKUP_STORAGE_LENGTH];
    char narrowAddress[NARROW_ADDRESS_BUFFER_LENGTH];

    if (std::wcscmp(pAddress, L"255.255.255.255") == 0)
    {
        m_address.sin_addr.s_addr = 0xFFFFFFFF;
        return true;
    }

    if (std::wcscmp(pAddress, L"localhost") == 0)
    {
        return SetLocalHost();
    }

    StringConversion::T2Char8(pAddress, narrowAddress, NARROW_ADDRESS_BUFFER_LENGTH);
    m_address.sin_addr.s_addr = String2Address(narrowAddress);

    if (g_autoLookupEnabled && m_address.sin_addr.s_addr == 0xFFFFFFFF)
    {
        hostent* pHost = gethostbyname(narrowAddress);

        if (pHost == 0)
        {
            return false;
        }

        m_address.sin_addr.s_addr = *reinterpret_cast<unsigned int*>(pHost->h_addr_list[0]);
        GetAddress(resolvedAddress, LOOKUP_BUFFER_LENGTH);
    }

    return true;
}

void InetAddress::SetAddress(unsigned int uiAddress)
{
    m_address.sin_addr.s_addr = htonl(uiAddress);
}

void InetAddress::SetNetworkAddress(unsigned int uiAddress)
{
    m_address.sin_addr.s_addr = uiAddress;
}

unsigned short InetAddress::GetPortNumber() const
{
    return ntohs(m_address.sin_port);
}

bool InetAddress::GetPortNumber(wchar_t* pBuffer, unsigned int uiSize) const
{
    std::swprintf(pBuffer, uiSize, L"%d", GetPortNumber());
    return true;
}

String InetAddress::GetPortNumberStr() const
{
    wchar_t buffer[PORT_BUFFER_LENGTH];

    GetPortNumber(buffer, PORT_BUFFER_LENGTH);
    return String(buffer);
}

void InetAddress::SetPortNumber(unsigned short uiPortNumber)
{
    m_address.sin_port = htons(uiPortNumber);
}

bool InetAddress::SetLocalHost()
{
    return SetAddress(L"127.0.0.1");
}

void InetAddress::ToStr(wchar_t* pBuffer) const
{
    wchar_t addressBuffer[ADDRESS_BUFFER_LENGTH];
    wchar_t portBuffer[PORT_BUFFER_LENGTH];

    GetAddress(addressBuffer, ADDRESS_BUFFER_LENGTH);
    GetPortNumber(portBuffer, PORT_BUFFER_LENGTH);

    StringStream stream;
    stream << addressBuffer << L":" << GetPortNumberStr();

    std::wcscpy(pBuffer, stream.GetBuffer());
}

void InetAddress::Trace(unsigned long long) const
{
    wchar_t buffer[TRACE_BUFFER_LENGTH];

    ToStr(buffer);
}

unsigned int InetAddress::String2Address(const char* pAddress)
{
    return inet_addr(pAddress);
}

void InetAddress::EnableAutoLookup(bool bEnable)
{
    g_autoLookupEnabled = bEnable;
}

StringStream& operator<<(StringStream& rStream, const InetAddress& rAddress)
{
    wchar_t buffer[STREAM_BUFFER_LENGTH];

    rAddress.ToStr(buffer);
    rStream << buffer;
    return rStream;
}

}
}
