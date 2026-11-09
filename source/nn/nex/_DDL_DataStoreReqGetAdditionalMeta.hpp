#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETADDITIONALMETA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREREQGETADDITIONALMETA_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qBuffer.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreReqGetAdditionalMeta : public RootObject
{
public:
    _DDL_DataStoreReqGetAdditionalMeta() : m_uiStructureVersion(0) {}

    virtual ~_DDL_DataStoreReqGetAdditionalMeta() {}

    _DDL_DataStoreReqGetAdditionalMeta& operator=(const _DDL_DataStoreReqGetAdditionalMeta& rSource);

    bool operator==(const _DDL_DataStoreReqGetAdditionalMeta& rSource) const
    {
        return m_uiOwnerId == rSource.m_uiOwnerId && m_uiDataType == rSource.m_uiDataType
               && m_uiVersion == rSource.m_uiVersion && m_MetaBinary == rSource.m_MetaBinary;
    }

    static void Add(Message* pMessage, const _DDL_DataStoreReqGetAdditionalMeta& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreReqGetAdditionalMeta* pDestination);

    uint8_t m_uiStructureVersion;
    uint32_t m_uiOwnerId;
    uint16_t m_uiDataType;
    uint16_t m_uiVersion;
    qBuffer m_MetaBinary;
};

}
}

#endif
