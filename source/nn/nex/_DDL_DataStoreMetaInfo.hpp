#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREMETAINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_DATASTOREMETAINFO_HPP

#include <stdint.h>

#include "nn/nex/DataStorePermission.hpp"
#include "nn/nex/DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qBuffer.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_DataStoreMetaInfo : public RootObject
{
public:
    _DDL_DataStoreMetaInfo() : m_uiStructureVersion(0), m_Tags(0), m_Ratings(0) {}

    virtual ~_DDL_DataStoreMetaInfo() {}

    _DDL_DataStoreMetaInfo& operator=(const _DDL_DataStoreMetaInfo& rSource);
    bool operator==(const _DDL_DataStoreMetaInfo& rSource) const;

    static void Add(Message* pMessage, const _DDL_DataStoreMetaInfo& rSource);
    static void Extract(Message* pMessage, _DDL_DataStoreMetaInfo* pDestination);

    uint8_t m_uiStructureVersion;
    uint64_t m_uiDataId;
    uint32_t m_uiOwnerId;
    uint32_t m_uiSize;
    String m_Name;
    uint16_t m_uiDataType;
    qBuffer m_MetaBinary;
    DataStorePermission m_Permission;
    DataStorePermission m_DelPermission;
    DateTime m_CreatedTime;
    DateTime m_UpdatedTime;
    uint16_t m_uiPeriod;
    uint8_t m_uiStatus;
    uint32_t m_uiReferredCnt;
    uint32_t m_uiReferDataId;
    uint32_t m_uiFlag;
    DateTime m_ReferredTime;
    DateTime m_ExpireTime;
    qVector<String> m_Tags;
    qVector<DataStoreRatingInfoWithSlot> m_Ratings;
};

}
}

#endif
