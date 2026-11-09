#include "nn/nex/_DDL_DataStorePersistenceInfo.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t PERSISTENCE_INFO_MAX_STRUCTURE_VERSION = 0;
const unsigned int PERSISTENCE_INFO_SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStorePersistenceInfo& _DDL_DataStorePersistenceInfo::operator=(const _DDL_DataStorePersistenceInfo& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiOwnerId = rSource.m_uiOwnerId;
    m_uiPersistenceSlotId = rSource.m_uiPersistenceSlotId;
    m_uiDataId = rSource.m_uiDataId;
    return *this;
}

void _DDL_DataStorePersistenceInfo::Add(Message* pMessage, const _DDL_DataStorePersistenceInfo& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = PERSISTENCE_INFO_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiOwnerId), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiPersistenceSlotId), 2, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiDataId), 8, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - PERSISTENCE_INFO_SIZE_FIELD_LENGTH);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStorePersistenceInfo::Extract(Message* pMessage, _DDL_DataStorePersistenceInfo* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiOwnerId);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiPersistenceSlotId);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiDataId);

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
