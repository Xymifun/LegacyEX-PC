#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_LOGINDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_LOGINDATA_HPP

#include <stdint.h>

#include "nn/nex/Data.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_LoginData : public Data
{
public:
    _DDL_LoginData() : Data(), m_uiStructureVersion(0), m_Name() {}

    virtual ~_DDL_LoginData() {}

    _DDL_LoginData& operator=(const _DDL_LoginData& rSource);
    bool operator==(const _DDL_LoginData& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_LoginData& rSource);
    static void Extract(Message* pMessage, _DDL_LoginData* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    signed char m_iValue;
    String m_Name;
    uint64_t m_uiValue64;
    uint32_t m_uiValue;
};

}
}

#endif
