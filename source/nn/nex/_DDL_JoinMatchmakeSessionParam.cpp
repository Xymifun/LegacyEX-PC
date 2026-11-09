#include "nn/nex/_DDL_JoinMatchmakeSessionParam.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 1;

}

_DDL_JoinMatchmakeSessionParam& _DDL_JoinMatchmakeSessionParam::operator=(const _DDL_JoinMatchmakeSessionParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiGatheringID = rSource.m_uiGatheringID;
    m_AdditionalParticipants = rSource.m_AdditionalParticipants;
    m_uiGatheringIDForParticipationCheck = rSource.m_uiGatheringIDForParticipationCheck;
    m_uiJoinMatchmakeSessionOption = rSource.m_uiJoinMatchmakeSessionOption;
    m_uiJoinMatchmakeSessionBehavior = rSource.m_uiJoinMatchmakeSessionBehavior;
    m_strUserPassword = rSource.m_strUserPassword;
    m_strSystemPassword = rSource.m_strSystemPassword;
    m_strJoinMessage = rSource.m_strJoinMessage;
    m_uiParticipationCount = rSource.m_uiParticipationCount;
    m_uiExtraParticipants = rSource.m_uiExtraParticipants;
    return *this;
}

void _DDL_JoinMatchmakeSessionParam::Add(Message* pMessage, const _DDL_JoinMatchmakeSessionParam& rSource)
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
    _Type_qlist<_Type_uint32>::Add(pMessage, rSource.m_AdditionalParticipants);
    _Type_uint32::Add(pMessage, rSource.m_uiGatheringIDForParticipationCheck);
    _Type_uint32::Add(pMessage, rSource.m_uiJoinMatchmakeSessionOption);
    _Type_byte::Add(pMessage, rSource.m_uiJoinMatchmakeSessionBehavior);
    _Type_string::Add(pMessage, rSource.m_strUserPassword);
    _Type_string::Add(pMessage, rSource.m_strSystemPassword);
    _Type_string::Add(pMessage, rSource.m_strJoinMessage);
    _Type_uint16::Add(pMessage, rSource.m_uiParticipationCount);
    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion != 0)
        {
            _Type_uint16::Add(pMessage, rSource.m_uiExtraParticipants);
        }
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_JoinMatchmakeSessionParam::Extract(Message* pMessage, _DDL_JoinMatchmakeSessionParam* pDestination)
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
    _Type_qlist<_Type_uint32>::Extract(pMessage, &pDestination->m_AdditionalParticipants);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiGatheringIDForParticipationCheck);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiJoinMatchmakeSessionOption);
    if (pMessage->HasError())
        return;
    _Type_byte::Extract(pMessage, &pDestination->m_uiJoinMatchmakeSessionBehavior);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strUserPassword);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strSystemPassword);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strJoinMessage);
    if (pMessage->HasError())
        return;
    _Type_uint16::Extract(pMessage, &pDestination->m_uiParticipationCount);
    if (pMessage->HasError())
        return;
    if (pDestination->m_uiStructureVersion != 0)
    {
        _Type_uint16::Extract(pMessage, &pDestination->m_uiExtraParticipants);
        if (pMessage->HasError())
            return;
    }
    if (bCompactFormat)
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
