#include "nn/nex/_DDL_DeletionEntry.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

void _DDL_DeletionEntry::Add(Message* pMessage, const _DDL_DeletionEntry& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = 0;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiValue1);
    _Type_uint32::Add(pMessage, rSource.m_uiValue2);
    _Type_uint32::Add(pMessage, rSource.m_uiValue3);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_DeletionEntry::Extract(Message* pMessage, _DDL_DeletionEntry* pDestination)
{
    pDestination->m_uiStructureVersion = 0;

    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;

    if (!bCompactFormat)
    {
        _Type_byte::Extract(pMessage, &pDestination->m_uiStructureVersion);
        if (pMessage->HasError())
        {
            return;
        }

        _Type_uint32::Extract(pMessage, &uiSize);
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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue1);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue2);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue3);
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
