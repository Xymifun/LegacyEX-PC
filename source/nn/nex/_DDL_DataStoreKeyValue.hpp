#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREKEYVALUE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREKEYVALUE_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreKeyValue : public RootObject
{
public:
    _DDL_DataStoreKeyValue() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreKeyValue() {}

    _DDL_DataStoreKeyValue& operator=(const _DDL_DataStoreKeyValue& rSource);
    bool operator==(const _DDL_DataStoreKeyValue& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreKeyValue& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreKeyValue* pDestination);

    uint8_t m_uiStructureVersion;
    String m_Key;
    String m_Value;
};

}
}

#endif
