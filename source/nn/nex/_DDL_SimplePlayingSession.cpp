#include "nn/nex/_DDL_SimplePlayingSession.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 0;

}

_DDL_SimplePlayingSession& _DDL_SimplePlayingSession::operator=(const _DDL_SimplePlayingSession& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiPrincipalID = rSource.m_uiPrincipalID;
    m_uiGatheringID = rSource.m_uiGatheringID;
    m_uiValue1 = rSource.m_uiValue1;
    m_uiValue2 = rSource.m_uiValue2;
    return *this;
}

void _DDL_SimplePlayingSession::Add(Message* pMessage, const _DDL_SimplePlayingSession& rSource)
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
    _Type_uint32::Add(pMessage, rSource.m_uiPrincipalID);
    _Type_uint32::Add(pMessage, rSource.m_uiGatheringID);
    _Type_uint32::Add(pMessage, rSource.m_uiValue1);
    _Type_uint32::Add(pMessage, rSource.m_uiValue2);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_SimplePlayingSession::Extract(Message* pMessage, _DDL_SimplePlayingSession* pDestination)
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
    _Type_uint32::Extract(pMessage, &pDestination->m_uiPrincipalID);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiGatheringID);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue1);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue2);
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
