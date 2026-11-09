#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DYNAMICDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DYNAMICDATA_HPP

#include <stdint.h>

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Data.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DynamicData : public Data
{
public:
    _DDL_DynamicData() : Data(), m_uiDynamicStructureVersion(0), m_Buffer() {}

    virtual ~_DDL_DynamicData() {}

    _DDL_DynamicData& operator=(const _DDL_DynamicData& rSource);
    bool operator==(const _DDL_DynamicData& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_DynamicData& rSource);
    static void Extract(Message* pMessage, _DDL_DynamicData* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiDynamicStructureVersion;
    Buffer m_Buffer;
};

}
}

#endif
