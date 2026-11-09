#include "nn/nex/_DDL_DataStoreSpecificMetaInfo.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t SPECIFIC_META_INFO_MAX_STRUCTURE_VERSION = 0;
const unsigned int SPECIFIC_META_INFO_SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStoreSpecificMetaInfo& _DDL_DataStoreSpecificMetaInfo::operator=(const _DDL_DataStoreSpecificMetaInfo& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiDataId = rSource.m_uiDataId;
    m_uiOwnerId = rSource.m_uiOwnerId;
    m_uiSize = rSource.m_uiSize;
    m_uiDataType = rSource.m_uiDataType;
    m_uiVersion = rSource.m_uiVersion;
    return *this;
}

void _DDL_DataStoreSpecificMetaInfo::Add(Message* pMessage, const _DDL_DataStoreSpecificMetaInfo& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = SPECIFIC_META_INFO_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiOwnerId), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiSize), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataType), 2, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiVersion), 4, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - SPECIFIC_META_INFO_SIZE_FIELD_LENGTH);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreSpecificMetaInfo::Extract(Message* pMessage, _DDL_DataStoreSpecificMetaInfo* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiDataType);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiVersion);

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
