#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATA_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Data;
class Message;

class _DDL_Data : public RootObject
{
public:
    _DDL_Data() : m_uiStructureVersion(0) {}

    virtual ~_DDL_Data() {}

    _DDL_Data& operator=(const _DDL_Data& rSource);
    bool operator==(const _DDL_Data& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_Data& rSource);
    static void Extract(Message* pMessage, _DDL_Data* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
};

}
}

#endif
