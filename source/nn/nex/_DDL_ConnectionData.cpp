#include "nn/nex/_DDL_ConnectionData.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t CONNECTION_DATA_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_ConnectionData& _DDL_ConnectionData::operator=(const _DDL_ConnectionData& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_StationURL = rSource.m_StationURL;
    m_uiValue = rSource.m_uiValue;
    return *this;
}

bool _DDL_ConnectionData::operator==(const _DDL_ConnectionData& rSource) const
{
    return m_StationURL == rSource.m_StationURL && m_uiValue == rSource.m_uiValue;
}

void _DDL_ConnectionData::Add(Message* pMessage, const _DDL_ConnectionData& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = CONNECTION_DATA_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_stationurl::Add(pMessage, rSource.m_StationURL);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiValue), 4, 1);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_ConnectionData::Extract(Message* pMessage, _DDL_ConnectionData* pDestination)
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

    _Type_stationurl::Extract(pMessage, &pDestination->m_StationURL);

    if (pMessage->HasError())
    {
        return;
    }

    pMessage->ExtractPrimitive(&pDestination->m_uiValue);

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
