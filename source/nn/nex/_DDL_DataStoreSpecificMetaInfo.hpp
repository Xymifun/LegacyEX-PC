#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESPECIFICMETAINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTORESPECIFICMETAINFO_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreSpecificMetaInfo : public RootObject
{
public:
    _DDL_DataStoreSpecificMetaInfo() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreSpecificMetaInfo() {}

    _DDL_DataStoreSpecificMetaInfo& operator=(const _DDL_DataStoreSpecificMetaInfo& rSource);
    bool operator==(const _DDL_DataStoreSpecificMetaInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreSpecificMetaInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreSpecificMetaInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiOwnerId;
    uint32_t m_uiSize;
    uint16_t m_uiDataType;
    uint32_t m_uiVersion;
};

}
}

#endif
