#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GAMEKEY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_GAMEKEY_HPP

#include <stdint.h>

#include "nn/nex/Data.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_GameKey : public Data
{
public:
    _DDL_GameKey() : Data(), m_uiStructureVersion(0) {}

    virtual ~_DDL_GameKey() {}

    _DDL_GameKey& operator=(const _DDL_GameKey& rSource);
    bool operator==(const _DDL_GameKey& rSource) const;

    virtual Data* Clone() const;
    virtual String GetDataType() const;
    virtual bool IsA(const String& rType) const;
    virtual bool IsAKindOf(const String& rType) const;
    virtual void StreamIn(Message* pMessage) const;
    virtual void StreamOut(Message* pMessage);

    static void Add(Message* pMessage, const _DDL_GameKey& rSource);
    static void Extract(Message* pMessage, _DDL_GameKey* pDestination);
    static Data* DynamicCast(Data* pData);
    static const Data* DynamicCast(const Data* pData);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiTitleID;
    uint16_t m_uiTitleVersion;
};

}
}

#endif
