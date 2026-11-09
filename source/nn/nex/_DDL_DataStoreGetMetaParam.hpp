#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETMETAPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETMETAPARAM_HPP

#include <stdint.h>

#include "nn/nex/DataStorePersistenceTarget.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreGetMetaParam : public RootObject
{
public:
    _DDL_DataStoreGetMetaParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreGetMetaParam() {}

    _DDL_DataStoreGetMetaParam& operator=(const _DDL_DataStoreGetMetaParam& rSource);

    bool operator==(const _DDL_DataStoreGetMetaParam& rSource) const
    {
        return m_uiDataId == rSource.m_uiDataId && m_PersistenceTarget == rSource.m_PersistenceTarget
               && m_uiResultOption == rSource.m_uiResultOption && m_uiAccessPassword == rSource.m_uiAccessPassword;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreGetMetaParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreGetMetaParam* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    DataStorePersistenceTarget m_PersistenceTarget;
    uint8_t m_uiResultOption;
    uint64_t m_uiAccessPassword;
};

}
}

#endif
