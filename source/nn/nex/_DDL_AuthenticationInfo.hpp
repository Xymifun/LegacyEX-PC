#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_AUTHENTICATIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_AUTHENTICATIONINFO_HPP

#include <stdint.h>

#include "nn/nex/Data.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_AuthenticationInfo : public Data
{
public:
    _DDL_AuthenticationInfo() : Data(), m_uiStructureVersion(0), m_Name() {}

    virtual ~_DDL_AuthenticationInfo() {}

    _DDL_AuthenticationInfo& operator=(const _DDL_AuthenticationInfo& rSource);
    bool operator==(const _DDL_AuthenticationInfo& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_AuthenticationInfo& rSource);
    static void Extract(Message* pMessage, _DDL_AuthenticationInfo* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    String m_Name;
    uint32_t m_uiValue1;
    uint8_t m_uiValue2;
    uint32_t m_uiValue3;
};

}
}

#endif
