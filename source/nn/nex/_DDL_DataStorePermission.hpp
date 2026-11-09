#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERMISSION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREPERMISSION_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStorePermission : public RootObject
{
public:
    _DDL_DataStorePermission() : m_uiStructureVersion(0), m_RecipientIds(0) {}

    virtual ~_DDL_DataStorePermission() {}

    _DDL_DataStorePermission& operator=(const _DDL_DataStorePermission& rSource);
    bool operator==(const _DDL_DataStorePermission& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStorePermission& rSource);
    static void Extract(Message* pMessage, _DDL_DataStorePermission* pDestination);

    uint8_t m_uiStructureVersion;
    uint8_t m_uiPermission;
    qVector<uint32_t> m_RecipientIds;
};

}
}

#endif
