#include "nn/nex/_DDL_ResultRange.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RESULT_RANGE_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_ResultRange& _DDL_ResultRange::operator=(const _DDL_ResultRange& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiOffset = rSource.m_uiOffset;
    m_uiSize = rSource.m_uiSize;
    return *this;
}

void _DDL_ResultRange::Add(Message* pMessage, const _DDL_ResultRange& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = RESULT_RANGE_MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiOffset), 4, 1);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiSize), 4, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_ResultRange::Extract(Message* pMessage, _DDL_ResultRange* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_uiOffset);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiSize);

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
