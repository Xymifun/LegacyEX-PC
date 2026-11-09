#include "nn/nex/_DDL_GatheringURLs.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 0;

}

_DDL_GatheringURLs& _DDL_GatheringURLs::operator=(const _DDL_GatheringURLs& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiGatheringID = rSource.m_uiGatheringID;
    m_StationURLs = rSource.m_StationURLs;
    return *this;
}

void _DDL_GatheringURLs::Add(Message* pMessage, const _DDL_GatheringURLs& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();
    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();
        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }
    _Type_uint32::Add(pMessage, rSource.m_uiGatheringID);
    _Type_std_list<_Type_stationurl>::Add(pMessage, rSource.m_StationURLs);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_GatheringURLs::Extract(Message* pMessage, _DDL_GatheringURLs* pDestination)
{
    pDestination->m_uiStructureVersion = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;
    if (!bCompactFormat)
    {
        _Type_byte::Extract(pMessage, &pDestination->m_uiStructureVersion);
        if (pMessage->HasError())
            return;
        _Type_uint32::Extract(pMessage, &uiSize);
        if (pMessage->HasError())
            return;
    }
    const unsigned int uiStartPosition = pMessage->GetPosition();
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiGatheringID);
    if (pMessage->HasError())
        return;
    _Type_std_list<_Type_stationurl>::Extract(pMessage, &pDestination->m_StationURLs);
    if (pMessage->HasError() || bCompactFormat)
        return;
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
