#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INTERFACEINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INTERFACEINFO_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class InterfaceInfo : public RootObject
{
public:
    enum Flags
    {
        FLAG_POINT2POINT = 1,
        FLAG_BROADCAST = 2,
        FLAG_LOOPBACK = 4
    };

    InterfaceInfo();
    ~InterfaceInfo();

    void SetAddress(unsigned int uiAddress);
    void SetBroadcastAddress(unsigned int uiAddress);
    void SetMask(unsigned int uiMask);

    bool Addr2Str(unsigned int uiAddress, wchar_t* pBuffer, unsigned int uiSize);

    unsigned int GetAddress();
    bool GetAddress(wchar_t* pBuffer, unsigned int uiSize);
    unsigned int GetBroadcastAddress();
    bool GetBroadcastAddress(wchar_t* pBuffer, unsigned int uiSize);
    bool GetMask(wchar_t* pBuffer, unsigned int uiSize);
    bool GetName(wchar_t* pBuffer, unsigned int uiSize);
    unsigned int GetFlags();
    bool GetFlags(wchar_t* pBuffer, unsigned int uiSize);

    void Trace(unsigned long long uiFilter);

private:
    unsigned int m_uiAddress;
    unsigned int m_uiBroadcastAddress;
    unsigned int m_uiMask;
    unsigned int m_uiFlags;
    String m_name;
};

}
}

#endif
