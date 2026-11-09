#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INETADDRESS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INETADDRESS_HPP

#include <netinet/in.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class StringStream;

class InetAddress : public RootObject
{
public:
    InetAddress();
    InetAddress(const InetAddress& rAddress);
    InetAddress(unsigned int uiAddress, unsigned short uiPortNumber);
    InetAddress(const wchar_t* pAddress, unsigned short uiPortNumber);
    InetAddress(void* pSocketAddress, unsigned int uiSize);
    virtual ~InetAddress();

    InetAddress& operator=(const InetAddress& rAddress);

    bool operator==(const InetAddress& rAddress) const
    {
        return m_address.sin_addr.s_addr == rAddress.m_address.sin_addr.s_addr
               && m_address.sin_port == rAddress.m_address.sin_port;
    }

    bool operator<(const InetAddress& rAddress) const
    {
        int iAddress = m_address.sin_addr.s_addr;
        int iOtherAddress = rAddress.m_address.sin_addr.s_addr;
        unsigned short uiPortNumber = m_address.sin_port;
        unsigned short uiOtherPortNumber = rAddress.m_address.sin_port;

        return iAddress <= iOtherAddress && (iAddress < iOtherAddress || uiPortNumber < uiOtherPortNumber);
    }

    void Init();

    unsigned int GetAddress() const;
    bool GetAddress(wchar_t* pBuffer, unsigned int uiSize) const;
    String GetAddressStr() const;
    bool SetAddress(const wchar_t* pAddress);
    void SetAddress(unsigned int uiAddress);
    void SetNetworkAddress(unsigned int uiAddress);

    unsigned short GetPortNumber() const;
    bool GetPortNumber(wchar_t* pBuffer, unsigned int uiSize) const;
    String GetPortNumberStr() const;
    void SetPortNumber(unsigned short uiPortNumber);
    void SetNetworkPortNumber(unsigned short uiPortNumber);

    bool SetLocalHost();
    bool IsLocalHost() const;

    void ToStr(wchar_t* pBuffer) const;
    String ToStr() const;
    void Trace(unsigned long long uiFilter) const;

    static unsigned int String2Address(const char* pAddress);
    static void EnableAutoLookup(bool bEnable);

private:
    sockaddr_in m_address;
};

StringStream& operator<<(StringStream& rStream, const InetAddress& rAddress);

}
}

#endif
