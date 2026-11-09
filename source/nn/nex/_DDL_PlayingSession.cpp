#include "nn/nex/_DDL_PlayingSession.hpp"

#include "nn/nex/DDLClassMetaType.hpp"
#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 0;

typedef _Type_any<DDLClassMetaType<Gathering>, _Type_string> GatheringAny;

}

_DDL_PlayingSession& _DDL_PlayingSession::operator=(const _DDL_PlayingSession& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiPrincipalID = rSource.m_uiPrincipalID;
    m_Gathering = rSource.m_Gathering;
    return *this;
}

void _DDL_PlayingSession::Add(Message* pMessage, const _DDL_PlayingSession& rSource)
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
    GatheringAny::Add(pMessage, rSource.m_Gathering);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_PlayingSession::Extract(Message* pMessage, _DDL_PlayingSession* pDestination)
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
    GatheringAny::Extract(pMessage, &pDestination->m_Gathering);
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
