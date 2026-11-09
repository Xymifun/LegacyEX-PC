#include "nn/nex/_DDL_DataStoreRatingInitParamWithSlot.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RATING_INIT_PARAM_WITH_SLOT_MAX_STRUCTURE_VERSION = 0;

}

_DDL_DataStoreRatingInitParamWithSlot& _DDL_DataStoreRatingInitParamWithSlot::operator=(
    const _DDL_DataStoreRatingInitParamWithSlot& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_iSlot = rSource.m_iSlot;
    m_Param = rSource.m_Param;
    return *this;
}

void _DDL_DataStoreRatingInitParamWithSlot::Add(Message* pMessage, const _DDL_DataStoreRatingInitParamWithSlot& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = RATING_INIT_PARAM_WITH_SLOT_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iSlot), 1, 1);
    _DDL_DataStoreRatingInitParam::Add(pMessage, rSource.m_Param);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DataStoreRatingInitParamWithSlot::Extract(Message* pMessage,
                                                    _DDL_DataStoreRatingInitParamWithSlot* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_iSlot);

    if (pMessage->HasError())
    {
        return;
    }

    _DDL_DataStoreRatingInitParam::Extract(pMessage, &pDestination->m_Param);

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
