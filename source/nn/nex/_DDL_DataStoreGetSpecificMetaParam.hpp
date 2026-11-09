#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETSPECIFICMETAPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREGETSPECIFICMETAPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreGetSpecificMetaParam : public RootObject
{
public:
    _DDL_DataStoreGetSpecificMetaParam() : m_uiStructureVersion(0), m_DataIds(0) {}

    virtual ~_DDL_DataStoreGetSpecificMetaParam() {}

    _DDL_DataStoreGetSpecificMetaParam& operator=(const _DDL_DataStoreGetSpecificMetaParam& rSource);
    bool operator==(const _DDL_DataStoreGetSpecificMetaParam& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreGetSpecificMetaParam& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreGetSpecificMetaParam* pDestination);

    uint8_t m_uiStructureVersion;
    qVector<uint64_t> m_DataIds;
};

}
}

#endif
