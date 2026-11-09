#include "nn/nex/_DDL_DataStoreMetaInfo.hpp"

#include "nn/nex/DDLClassMetaType.hpp"
#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t META_INFO_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStoreMetaInfo& _DDL_DataStoreMetaInfo::operator=(const _DDL_DataStoreMetaInfo& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiDataId = rSource.m_uiDataId;
    m_uiOwnerId = rSource.m_uiOwnerId;
    m_uiSize = rSource.m_uiSize;
    m_Name = rSource.m_Name;
    m_uiDataType = rSource.m_uiDataType;
    m_MetaBinary = rSource.m_MetaBinary;
    m_Permission = rSource.m_Permission;
    m_DelPermission = rSource.m_DelPermission;
    m_CreatedTime = rSource.m_CreatedTime;
    m_UpdatedTime = rSource.m_UpdatedTime;
    m_uiPeriod = rSource.m_uiPeriod;
    m_uiStatus = rSource.m_uiStatus;
    m_uiReferredCnt = rSource.m_uiReferredCnt;
    m_uiReferDataId = rSource.m_uiReferDataId;
    m_uiFlag = rSource.m_uiFlag;
    m_ReferredTime = rSource.m_ReferredTime;
    m_ExpireTime = rSource.m_ExpireTime;
    m_Tags = rSource.m_Tags;
    m_Ratings = rSource.m_Ratings;
    return *this;
}

void _DDL_DataStoreMetaInfo::Add(Message* pMessage, const _DDL_DataStoreMetaInfo& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = META_INFO_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiOwnerId), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiSize), 4, 1);
    _Type_string::Add(pMessage, rSource.m_Name);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataType), 2, 1);
    _Type_qBuffer::Add(pMessage, rSource.m_MetaBinary);
    _DDL_DataStorePermission::Add(pMessage, rSource.m_Permission);
    _DDL_DataStorePermission::Add(pMessage, rSource.m_DelPermission);

    const uint64_t uiCreatedTime = rSource.m_CreatedTime;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiCreatedTime), 8, 1);

    const uint64_t uiUpdatedTime = rSource.m_UpdatedTime;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiUpdatedTime), 8, 1);

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiPeriod), 2, 1);
    pMessage->Append(&rSource.m_uiStatus, 1, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiReferredCnt), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiReferDataId), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiFlag), 4, 1);

    const uint64_t uiReferredTime = rSource.m_ReferredTime;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiReferredTime), 8, 1);

    const uint64_t uiExpireTime = rSource.m_ExpireTime;

    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiExpireTime), 8, 1);

    _Type_qvector<_Type_string>::Add(pMessage, rSource.m_Tags);
    _Type_qvector<DDLClassMetaType<DataStoreRatingInfoWithSlot> >::Add(pMessage, rSource.m_Ratings);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - SIZE_FIELD_LENGTH);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreMetaInfo::Extract(Message* pMessage, _DDL_DataStoreMetaInfo* pDestination)
{
    pDestination->m_uiStructureVersion = 0;

    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;

    if (!bCompactFormat)
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);

        if (pMessage->HasError())
        {
            return;
        }

        pMessage->ExtractPrimitive(&uiSize);

        if (pMessage->HasError())
        {
            return;
        }
    }

    const unsigned int uiStartPosition = pMessage->GetPosition();

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataId);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiOwnerId);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiSize);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_string::Extract(pMessage, &pDestination->m_Name);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataType);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qBuffer::Extract(pMessage, &pDestination->m_MetaBinary);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStorePermission::Extract(pMessage, &pDestination->m_Permission);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStorePermission::Extract(pMessage, &pDestination->m_DelPermission);

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiCreatedTime;

    pMessage->ExtractPrimitive(&uiCreatedTime);
    const DateTime createdTime(uiCreatedTime);
    pDestination->m_CreatedTime = createdTime;

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiUpdatedTime;

    pMessage->ExtractPrimitive(&uiUpdatedTime);
    const DateTime updatedTime(uiUpdatedTime);
    pDestination->m_UpdatedTime = updatedTime;

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiPeriod);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiStatus);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiReferredCnt);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiReferDataId);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiFlag);

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiReferredTime;

    pMessage->ExtractPrimitive(&uiReferredTime);
    const DateTime referredTime(uiReferredTime);
    pDestination->m_ReferredTime = referredTime;

    if (pMessage->HasError())
    {
        return;
    }

    uint64_t uiExpireTime;

    pMessage->ExtractPrimitive(&uiExpireTime);
    const DateTime expireTime(uiExpireTime);
    pDestination->m_ExpireTime = expireTime;

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<_Type_string>::Extract(pMessage, &pDestination->m_Tags);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_qvector<DDLClassMetaType<DataStoreRatingInfoWithSlot> >::Extract(pMessage, &pDestination->m_Ratings);

    if (pMessage->HasError() || bCompactFormat)
    {
        return;
    }

    const unsigned int uiEndPosition = uiStartPosition + uiSize;

    if (pMessage->GetPosition() <= uiEndPosition && uiEndPosition <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiEndPosition);
        return;
    }

    pMessage->SetError();
}

}
}
