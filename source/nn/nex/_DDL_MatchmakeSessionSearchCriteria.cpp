#include "nn/nex/_DDL_MatchmakeSessionSearchCriteria.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
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

_DDL_MatchmakeSessionSearchCriteria& _DDL_MatchmakeSessionSearchCriteria::operator=(
    const _DDL_MatchmakeSessionSearchCriteria& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_Attributes = rSource.m_Attributes;
    m_strGameMode = rSource.m_strGameMode;
    m_strMinParticipants = rSource.m_strMinParticipants;
    m_strMaxParticipants = rSource.m_strMaxParticipants;
    m_strMatchmakeSystemType = rSource.m_strMatchmakeSystemType;
    m_bVacantOnly = rSource.m_bVacantOnly;
    m_bExcludeLocked = rSource.m_bExcludeLocked;
    m_bExcludeNonHostPid = rSource.m_bExcludeNonHostPid;
    m_uiSelectionMethod = rSource.m_uiSelectionMethod;
    m_uiVacantParticipants = rSource.m_uiVacantParticipants;
    m_MatchmakeParam = rSource.m_MatchmakeParam;
    m_bExcludeUserPasswordSet = rSource.m_bExcludeUserPasswordSet;
    m_bExcludeSystemPasswordSet = rSource.m_bExcludeSystemPasswordSet;
    m_uiReferGatheringID = rSource.m_uiReferGatheringID;
    return *this;
}

void _DDL_MatchmakeSessionSearchCriteria::Add(Message* pMessage, const _DDL_MatchmakeSessionSearchCriteria& rSource)
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
    _Type_qvector<_Type_string>::Add(pMessage, rSource.m_Attributes);
    _Type_string::Add(pMessage, rSource.m_strGameMode);
    _Type_string::Add(pMessage, rSource.m_strMinParticipants);
    _Type_string::Add(pMessage, rSource.m_strMaxParticipants);
    _Type_string::Add(pMessage, rSource.m_strMatchmakeSystemType);
    *pMessage << rSource.m_bVacantOnly;
    *pMessage << rSource.m_bExcludeLocked;
    *pMessage << rSource.m_bExcludeNonHostPid;
    _Type_uint32::Add(pMessage, rSource.m_uiSelectionMethod);
    _Type_uint16::Add(pMessage, rSource.m_uiVacantParticipants);
    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion != 0)
        {
            _DDL_MatchmakeParam::Add(pMessage, rSource.m_MatchmakeParam);
            if (rSource.m_uiStructureVersion >= 2)
            {
                *pMessage << rSource.m_bExcludeUserPasswordSet;
                *pMessage << rSource.m_bExcludeSystemPasswordSet;
                if (rSource.m_uiStructureVersion >= 3)
                {
                    _Type_uint32::Add(pMessage, rSource.m_uiReferGatheringID);
                }
            }
        }
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_MatchmakeSessionSearchCriteria::Extract(Message* pMessage, _DDL_MatchmakeSessionSearchCriteria* pDestination)
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
    _Type_qvector<_Type_string>::Extract(pMessage, &pDestination->m_Attributes);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strGameMode);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strMinParticipants);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strMaxParticipants);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strMatchmakeSystemType);
    if (pMessage->HasError())
        return;
    *pMessage >> pDestination->m_bVacantOnly;
    if (pMessage->HasError())
        return;
    *pMessage >> pDestination->m_bExcludeLocked;
    if (pMessage->HasError())
        return;
    *pMessage >> pDestination->m_bExcludeNonHostPid;
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiSelectionMethod);
    if (pMessage->HasError())
        return;
    _Type_uint16::Extract(pMessage, &pDestination->m_uiVacantParticipants);
    if (pMessage->HasError())
        return;
    if (pDestination->m_uiStructureVersion != 0)
    {
        _DDL_MatchmakeParam::Extract(pMessage, &pDestination->m_MatchmakeParam);
        if (pMessage->HasError())
            return;
        if (pDestination->m_uiStructureVersion >= 2)
        {
            *pMessage >> pDestination->m_bExcludeUserPasswordSet;
            if (pMessage->HasError())
                return;
            *pMessage >> pDestination->m_bExcludeSystemPasswordSet;
            if (pMessage->HasError())
                return;
            if (pDestination->m_uiStructureVersion >= 3)
            {
                _Type_uint32::Extract(pMessage, &pDestination->m_uiReferGatheringID);
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
