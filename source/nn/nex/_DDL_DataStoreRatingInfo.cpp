#include "nn/nex/_DDL_DataStoreRatingInfo.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RATING_INFO_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_DataStoreRatingInfo& _DDL_DataStoreRatingInfo::operator=(const _DDL_DataStoreRatingInfo& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_iTotalValue = rSource.m_iTotalValue;
    m_uiCount = rSource.m_uiCount;
    m_iInitialValue = rSource.m_iInitialValue;
    return *this;
}

void _DDL_DataStoreRatingInfo::Add(Message* pMessage, const _DDL_DataStoreRatingInfo& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = RATING_INFO_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iTotalValue), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiCount), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iInitialValue), 8, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - SIZE_FIELD_LENGTH);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreRatingInfo::Extract(Message* pMessage, _DDL_DataStoreRatingInfo* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_iTotalValue);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiCount);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iInitialValue);

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
