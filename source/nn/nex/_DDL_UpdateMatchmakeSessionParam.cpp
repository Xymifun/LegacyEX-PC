#include "nn/nex/_DDL_UpdateMatchmakeSessionParam.hpp"

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

const uint8_t MAX_STRUCTURE_VERSION = 1;

}

_DDL_UpdateMatchmakeSessionParam& _DDL_UpdateMatchmakeSessionParam::operator=(
    const _DDL_UpdateMatchmakeSessionParam& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiValue1 = rSource.m_uiValue1;
    m_uiValue2 = rSource.m_uiValue2;
    m_Attributes = rSource.m_Attributes;
    m_bOpenParticipation = rSource.m_bOpenParticipation;
    m_ApplicationBuffer = rSource.m_ApplicationBuffer;
    m_uiProgressScore = rSource.m_uiProgressScore;
    m_MatchmakeParam = rSource.m_MatchmakeParam;
    m_StartedTime = rSource.m_StartedTime;
    m_strValue1 = rSource.m_strValue1;
    m_uiValue3 = rSource.m_uiValue3;
    m_strValue2 = rSource.m_strValue2;
    m_uiValue4 = rSource.m_uiValue4;
    m_uiValue5 = rSource.m_uiValue5;
    m_uiValue6 = rSource.m_uiValue6;
    m_uiValue7 = rSource.m_uiValue7;
    m_uiValue8 = rSource.m_uiValue8;
    return *this;
}

void _DDL_UpdateMatchmakeSessionParam::Add(Message* pMessage, const _DDL_UpdateMatchmakeSessionParam& rSource)
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
    _Type_uint32::Add(pMessage, rSource.m_uiValue1);
    _Type_uint32::Add(pMessage, rSource.m_uiValue2);
    _Type_qvector<_Type_uint32>::Add(pMessage, rSource.m_Attributes);
    *pMessage << rSource.m_bOpenParticipation;
    _Type_qvector<_Type_byte>::Add(pMessage, rSource.m_ApplicationBuffer);
    _Type_byte::Add(pMessage, rSource.m_uiProgressScore);
    _DDL_MatchmakeParam::Add(pMessage, rSource.m_MatchmakeParam);
    const unsigned long long uiStartedTime = rSource.m_StartedTime;
    _Type_uint64::Add(pMessage, uiStartedTime);
    _Type_string::Add(pMessage, rSource.m_strValue1);
    _Type_uint32::Add(pMessage, rSource.m_uiValue3);
    _Type_string::Add(pMessage, rSource.m_strValue2);
    _Type_uint16::Add(pMessage, rSource.m_uiValue4);
    _Type_uint16::Add(pMessage, rSource.m_uiValue5);
    if (!bCompactFormat)
    {
        if (rSource.m_uiStructureVersion != 0)
        {
            _Type_uint32::Add(pMessage, rSource.m_uiValue6);
            _Type_uint32::Add(pMessage, rSource.m_uiValue7);
            _Type_uint32::Add(pMessage, rSource.m_uiValue8);
        }
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_UpdateMatchmakeSessionParam::Extract(Message* pMessage, _DDL_UpdateMatchmakeSessionParam* pDestination)
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
    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue1);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue2);
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_uint32>::Extract(pMessage, &pDestination->m_Attributes);
    if (pMessage->HasError())
        return;
    *pMessage >> pDestination->m_bOpenParticipation;
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_byte>::Extract(pMessage, &pDestination->m_ApplicationBuffer);
    if (pMessage->HasError())
        return;
    _Type_byte::Extract(pMessage, &pDestination->m_uiProgressScore);
    if (pMessage->HasError())
        return;
    _DDL_MatchmakeParam::Extract(pMessage, &pDestination->m_MatchmakeParam);
    if (pMessage->HasError())
        return;
    unsigned long long uiStartedTime;
    _Type_uint64::Extract(pMessage, &uiStartedTime);
    pDestination->m_StartedTime = DateTime(uiStartedTime);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strValue1);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue3);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strValue2);
    if (pMessage->HasError())
        return;
    _Type_uint16::Extract(pMessage, &pDestination->m_uiValue4);
    if (pMessage->HasError())
        return;
    _Type_uint16::Extract(pMessage, &pDestination->m_uiValue5);
    if (pMessage->HasError())
        return;
    if (pDestination->m_uiStructureVersion != 0)
    {
        _Type_uint32::Extract(pMessage, &pDestination->m_uiValue6);
        if (pMessage->HasError())
            return;
        _Type_uint32::Extract(pMessage, &pDestination->m_uiValue7);
        if (pMessage->HasError())
            return;
        _Type_uint32::Extract(pMessage, &pDestination->m_uiValue8);
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
