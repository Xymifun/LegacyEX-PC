#include "nn/nex/InterfaceInfo.hpp"

#include <cwchar>

#include "nn/nex/BerkeleySocketDriver.hpp"
#include "nn/nex/StringConversion.hpp"

namespace nn
{
namespace nex
{

InterfaceInfo::InterfaceInfo()
{
    m_uiAddress = 0;
    m_uiBroadcastAddress = 0;
    m_uiMask = 0;
    m_uiFlags = 0;
}

InterfaceInfo::~InterfaceInfo()
{
}

void InterfaceInfo::SetAddress(unsigned int uiAddress)
{
    m_uiAddress = uiAddress;
}

void InterfaceInfo::SetBroadcastAddress(unsigned int uiAddress)
{
    m_uiBroadcastAddress = uiAddress;
}

void InterfaceInfo::SetMask(unsigned int uiMask)
{
    m_uiMask = uiMask;
}

bool InterfaceInfo::Addr2Str(unsigned int uiAddress, wchar_t* pBuffer, unsigned int uiSize)
{
    in_addr address;

    address.s_addr = uiAddress;
    StringConversion::Char8_2T(inet_ntoa(address), pBuffer, uiSize);
    return true;
}

unsigned int InterfaceInfo::GetAddress()
{
    return m_uiAddress;
}

bool InterfaceInfo::GetAddress(wchar_t* pBuffer, unsigned int uiSize)
{
    return Addr2Str(m_uiAddress, pBuffer, uiSize);
}

unsigned int InterfaceInfo::GetBroadcastAddress()
{
    return m_uiBroadcastAddress;
}

bool InterfaceInfo::GetBroadcastAddress(wchar_t* pBuffer, unsigned int uiSize)
{
    return Addr2Str(m_uiBroadcastAddress, pBuffer, uiSize);
}

bool InterfaceInfo::GetMask(wchar_t* pBuffer, unsigned int uiSize)
{
    return Addr2Str(m_uiMask, pBuffer, uiSize);
}

bool InterfaceInfo::GetName(wchar_t* pBuffer, unsigned int uiSize)
{
    if (m_name == 0)
    {
        return false;
    }

    std::wcsncpy(pBuffer, m_name, uiSize);
    pBuffer[uiSize - 1] = L'\0';
    return true;
}

unsigned int InterfaceInfo::GetFlags()
{
    return m_uiFlags;
}

bool InterfaceInfo::GetFlags(wchar_t* pBuffer, unsigned int uiSize)
{
    wchar_t flags[512] = L"";

    if ((m_uiFlags & FLAG_POINT2POINT) != 0)
    {
        std::wcscat(flags, L"POINT2POINT");
    }

    if ((m_uiFlags & FLAG_LOOPBACK) != 0)
    {
        std::wcscat(flags, L"LOOPBACK ");
    }

    if ((m_uiFlags & FLAG_BROADCAST) != 0)
    {
        std::wcscat(flags, L"BROADCAST ");
    }

    std::wcsncpy(pBuffer, flags, uiSize);
    pBuffer[uiSize - 1] = L'\0';
    return true;
}

void InterfaceInfo::Trace(unsigned long long uiFilter)
{
    wchar_t buffer[128];

    GetName(buffer, 128);
    GetAddress(buffer, 128);
    GetBroadcastAddress(buffer, 128);
    GetMask(buffer, 128);
    GetFlags(buffer, 128);
}

}
}
