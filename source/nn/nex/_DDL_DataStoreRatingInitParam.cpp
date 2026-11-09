#include "nn/nex/_DDL_DataStoreRatingInitParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RATING_INIT_PARAM_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStoreRatingInitParam& _DDL_DataStoreRatingInitParam::operator=(const _DDL_DataStoreRatingInitParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiFlag = rSource.m_uiFlag;
    m_uiInternalFlag = rSource.m_uiInternalFlag;
    m_uiLockType = rSource.m_uiLockType;
    m_iInitialValue = rSource.m_iInitialValue;
    m_iRangeMin = rSource.m_iRangeMin;
    m_iRangeMax = rSource.m_iRangeMax;
    m_iPeriodHour = rSource.m_iPeriodHour;
    m_iPeriodDuration = rSource.m_iPeriodDuration;
    return *this;
}

void _DDL_DataStoreRatingInitParam::Add(Message* pMessage, const _DDL_DataStoreRatingInitParam& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = RATING_INIT_PARAM_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiFlag), 1, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiInternalFlag), 1, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiLockType), 1, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iInitialValue), 8, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iRangeMin), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iRangeMax), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iPeriodHour), 1, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iPeriodDuration), 2, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreRatingInitParam::Extract(Message* pMessage, _DDL_DataStoreRatingInitParam* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiFlag);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiInternalFlag);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiLockType);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iInitialValue);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iRangeMin);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iRangeMax);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iPeriodHour);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_iPeriodDuration);

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
