#include "nn/nex/_DDL_MatchmakeSession.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/_DDL_MatchmakeParam.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 3;

}

String _DDL_MatchmakeSession::GetGatheringType() const
{
    return String(L"MatchmakeSession");
}

bool _DDL_MatchmakeSession::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"MatchmakeSession");
}

bool _DDL_MatchmakeSession::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"MatchmakeSession") || Gathering::IsAKindOf(rType);
}

void _DDL_MatchmakeSession::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_MatchmakeSession::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

_DDL_MatchmakeSession& _DDL_MatchmakeSession::operator=(const _DDL_MatchmakeSession& rSource)
{
    Gathering::operator=(rSource);
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiGameMode = rSource.m_uiGameMode;
    m_Attributes = rSource.m_Attributes;
    m_bOpenParticipation = rSource.m_bOpenParticipation;
    m_uiMatchmakeSystemType = rSource.m_uiMatchmakeSystemType;
    m_ApplicationBuffer = rSource.m_ApplicationBuffer;
    m_uiParticipationCount = rSource.m_uiParticipationCount;
    m_uiProgressScore = rSource.m_uiProgressScore;
    m_SessionKey = rSource.m_SessionKey;
    m_uiOption = rSource.m_uiOption;
    m_MatchmakeParam = rSource.m_MatchmakeParam;
    m_StartedTime = rSource.m_StartedTime;
    m_strUserPassword = rSource.m_strUserPassword;
    m_uiReferGatheringID = rSource.m_uiReferGatheringID;
    m_bUserPasswordEnabled = rSource.m_bUserPasswordEnabled;
    m_bSystemPasswordEnabled = rSource.m_bSystemPasswordEnabled;
    return *this;
}

void _DDL_MatchmakeSession::Add(Message* pMessage, const _DDL_MatchmakeSession& rSource)
{
    _DDL_Gathering::Add(pMessage, rSource);
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
    _Type_uint32::Add(pMessage, rSource.m_uiGameMode);
    _Type_qvector<_Type_uint32>::Add(pMessage, rSource.m_Attributes);
    *pMessage << rSource.m_bOpenParticipation;
    _Type_uint32::Add(pMessage, rSource.m_uiMatchmakeSystemType);
    _Type_qvector<_Type_byte>::Add(pMessage, rSource.m_ApplicationBuffer);
    _Type_uint32::Add(pMessage, rSource.m_uiParticipationCount);
    _Type_byte::Add(pMessage, rSource.m_uiProgressScore);
    _Type_qvector<_Type_byte>::Add(pMessage, rSource.m_SessionKey);
    _Type_uint32::Add(pMessage, rSource.m_uiOption);
    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion != 0)
        {
            _DDL_MatchmakeParam::Add(pMessage, rSource.m_MatchmakeParam);
            const unsigned long long uiStartedTime = rSource.m_StartedTime;
            _Type_uint64::Add(pMessage, uiStartedTime);
            if (rSource.m_uiStructureVersion >= 2)
            {
                _Type_string::Add(pMessage, rSource.m_strUserPassword);
                if (rSource.m_uiStructureVersion >= 3)
                {
                    _Type_uint32::Add(pMessage, rSource.m_uiReferGatheringID);
                    *pMessage << rSource.m_bUserPasswordEnabled;
                    *pMessage << rSource.m_bSystemPasswordEnabled;
                }
            }
        }
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_MatchmakeSession::Extract(Message* pMessage, _DDL_MatchmakeSession* pDestination)
{
    _DDL_Gathering::Extract(pMessage, pDestination);
    if (pMessage->HasError())
        return;
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
    _Type_uint32::Extract(pMessage, &pDestination->m_uiGameMode);
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_uint32>::Extract(pMessage, &pDestination->m_Attributes);
    if (pMessage->HasError())
        return;
    *pMessage >> pDestination->m_bOpenParticipation;
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiMatchmakeSystemType);
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_byte>::Extract(pMessage, &pDestination->m_ApplicationBuffer);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiParticipationCount);
    if (pMessage->HasError())
        return;
    _Type_byte::Extract(pMessage, &pDestination->m_uiProgressScore);
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_byte>::Extract(pMessage, &pDestination->m_SessionKey);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiOption);
    if (pMessage->HasError())
        return;
    if (pDestination->m_uiStructureVersion != 0)
    {
        _DDL_MatchmakeParam::Extract(pMessage, &pDestination->m_MatchmakeParam);
        if (pMessage->HasError())
            return;
        unsigned long long uiStartedTime;
        _Type_uint64::Extract(pMessage, &uiStartedTime);
        pDestination->m_StartedTime = DateTime(uiStartedTime);
        if (pMessage->HasError())
            return;
        if (pDestination->m_uiStructureVersion >= 2)
        {
            _Type_string::Extract(pMessage, &pDestination->m_strUserPassword);
            if (pMessage->HasError())
                return;
            if (pDestination->m_uiStructureVersion >= 3)
            {
                _Type_uint32::Extract(pMessage, &pDestination->m_uiReferGatheringID);
                if (pMessage->HasError())
                    return;
                *pMessage >> pDestination->m_bUserPasswordEnabled;
                if (pMessage->HasError())
                    return;
                *pMessage >> pDestination->m_bSystemPasswordEnabled;
                if (pMessage->HasError())
                    return;
            }
        }
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
