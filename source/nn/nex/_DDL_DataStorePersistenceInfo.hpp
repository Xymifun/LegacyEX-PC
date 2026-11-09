#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERSISTENCEINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERSISTENCEINFO_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePersistenceInfo : public RootObject
{
public:
    _DDL_DataStorePersistenceInfo() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStorePersistenceInfo() {}

    _DDL_DataStorePersistenceInfo& operator=(const _DDL_DataStorePersistenceInfo& rSource);
    bool operator==(const _DDL_DataStorePersistenceInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePersistenceInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePersistenceInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiOwnerId;
    uint16_t m_uiPersistenceSlotId;
    uint64_t m_uiDataId;
};

}
}

#endif
