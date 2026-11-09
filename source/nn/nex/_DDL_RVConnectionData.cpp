#include "nn/nex/_DDL_RVConnectionData.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t RV_CONNECTION_DATA_MAX_STRUCTURE_VERSION = 1;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

void _DDL_RVConnectionData::Add(Message* pMessage, const _DDL_RVConnectionData& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = RV_CONNECTION_DATA_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_stationurl::Add(pMessage, rSource.m_RegularProtocolsURL);
    _Type_std_list<_Type_byte>::Add(pMessage, rSource.m_SpecialProtocols);
    _Type_stationurl::Add(pMessage, rSource.m_SpecialProtocolsURL);

    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion != 0)
        {
            const unsigned long long ullTime = rSource.m_Time;

            pMessage->Append(reinterpret_cast<const unsigned char*>(&ullTime), 8, 1);
        }

        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_RVConnectionData::Extract(Message* pMessage, _DDL_RVConnectionData* pDestination)
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

    _Type_stationurl::Extract(pMessage, &pDestination->m_RegularProtocolsURL);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_std_list<_Type_byte>::Extract(pMessage, &pDestination->m_SpecialProtocols);

    if (pMessage->HasError())
    {
        return;
    }

    _Type_stationurl::Extract(pMessage, &pDestination->m_SpecialProtocolsURL);

    if (pMessage->HasError())
    {
        return;
    }

    if (pDestination->m_uiStructureVersion != 0)
    {
        unsigned long long ullTime;

        pMessage->ExtractPrimitive(&ullTime);

        pDestination->m_Time = DateTime(ullTime);

        if (pMessage->HasError())
        {
            return;
        }
    }

    if (bCompactFormat)
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
