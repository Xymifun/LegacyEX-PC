#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERSISTENCETARGET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERSISTENCETARGET_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePersistenceTarget : public RootObject
{
public:
    static const uint16_t NO_PERSISTENCE_SLOT_ID = 0xFFFF;

    _DDL_DataStorePersistenceTarget()
        : m_uiStructureVersion(0),
          m_uiOwnerId(0),
          m_uiPersistenceSlotId(NO_PERSISTENCE_SLOT_ID)
    {
    }

    virtual ~_DDL_DataStorePersistenceTarget() {}

    _DDL_DataStorePersistenceTarget& operator=(const _DDL_DataStorePersistenceTarget& rSource);
    bool operator==(const _DDL_DataStorePersistenceTarget& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePersistenceTarget& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePersistenceTarget* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiOwnerId;
    uint16_t m_uiPersistenceSlotId;
};

}
}

#endif
