#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERSISTENCEINITPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERSISTENCEINITPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePersistenceInitParam : public RootObject
{
public:
    _DDL_DataStorePersistenceInitParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStorePersistenceInitParam() {}

    _DDL_DataStorePersistenceInitParam& operator=(const _DDL_DataStorePersistenceInitParam& rSource);
    bool operator==(const _DDL_DataStorePersistenceInitParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePersistenceInitParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePersistenceInitParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint16_t m_uiPersistenceSlotId;
    bool m_bDeleteLastObject;
};

}
}

#endif
