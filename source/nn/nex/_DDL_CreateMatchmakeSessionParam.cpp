#include "nn/nex/_DDL_CreateMatchmakeSessionParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/_DDL_MatchmakeSession.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 0;

}

_DDL_CreateMatchmakeSessionParam& _DDL_CreateMatchmakeSessionParam::operator=(
    const _DDL_CreateMatchmakeSessionParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_MatchmakeSession = rSource.m_MatchmakeSession;
    m_AdditionalParticipants = rSource.m_AdditionalParticipants;
    m_uiGatheringIDForParticipationCheck = rSource.m_uiGatheringIDForParticipationCheck;
    m_uiCreateMatchmakeSessionOption = rSource.m_uiCreateMatchmakeSessionOption;
    m_strJoinMessage = rSource.m_strJoinMessage;
    m_uiParticipationCount = rSource.m_uiParticipationCount;
    return *this;
}

void _DDL_CreateMatchmakeSessionParam::Add(Message* pMessage, const _DDL_CreateMatchmakeSessionParam& rSource)
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
    _DDL_MatchmakeSession::Add(pMessage, rSource.m_MatchmakeSession);
    _Type_qlist<_Type_uint32>::Add(pMessage, rSource.m_AdditionalParticipants);
    _Type_uint32::Add(pMessage, rSource.m_uiGatheringIDForParticipationCheck);
    _Type_uint32::Add(pMessage, rSource.m_uiCreateMatchmakeSessionOption);
    _Type_string::Add(pMessage, rSource.m_strJoinMessage);
    _Type_uint16::Add(pMessage, rSource.m_uiParticipationCount);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_CreateMatchmakeSessionParam::Extract(Message* pMessage, _DDL_CreateMatchmakeSessionParam* pDestination)
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
    _DDL_MatchmakeSession::Extract(pMessage, &pDestination->m_MatchmakeSession);
    if (pMessage->HasError())
        return;
    _Type_qlist<_Type_uint32>::Extract(pMessage, &pDestination->m_AdditionalParticipants);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiGatheringIDForParticipationCheck);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiCreateMatchmakeSessionOption);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strJoinMessage);
    if (pMessage->HasError())
        return;
    _Type_uint16::Extract(pMessage, &pDestination->m_uiParticipationCount);
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
