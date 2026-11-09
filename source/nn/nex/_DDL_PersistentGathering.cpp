#include "nn/nex/_DDL_PersistentGathering.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t MAX_STRUCTURE_VERSION = 0;

}

String _DDL_PersistentGathering::GetGatheringType() const
{
    return String(L"PersistentGathering");
}

bool _DDL_PersistentGathering::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"PersistentGathering");
}

bool _DDL_PersistentGathering::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"PersistentGathering") || Gathering::IsAKindOf(rType);
}

void _DDL_PersistentGathering::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_PersistentGathering::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

_DDL_PersistentGathering& _DDL_PersistentGathering::operator=(const _DDL_PersistentGathering& rSource)
{
    Gathering::operator=(rSource);
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiPersistentGatheringType = rSource.m_uiPersistentGatheringType;
    m_strPassword = rSource.m_strPassword;
    m_Attributes = rSource.m_Attributes;
    m_ApplicationBuffer = rSource.m_ApplicationBuffer;
    m_ParticipationStartDate = rSource.m_ParticipationStartDate;
    m_ParticipationEndDate = rSource.m_ParticipationEndDate;
    m_uiMatchmakeSessionCount = rSource.m_uiMatchmakeSessionCount;
    m_uiParticipationCount = rSource.m_uiParticipationCount;
    return *this;
}

void _DDL_PersistentGathering::Add(Message* pMessage, const _DDL_PersistentGathering& rSource)
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
    _Type_uint32::Add(pMessage, rSource.m_uiPersistentGatheringType);
    _Type_string::Add(pMessage, rSource.m_strPassword);
    _Type_qvector<_Type_uint32>::Add(pMessage, rSource.m_Attributes);
    _Type_qvector<_Type_byte>::Add(pMessage, rSource.m_ApplicationBuffer);
    const unsigned long long uiParticipationStartDate = rSource.m_ParticipationStartDate;
    _Type_uint64::Add(pMessage, uiParticipationStartDate);
    const unsigned long long uiParticipationEndDate = rSource.m_ParticipationEndDate;
    _Type_uint64::Add(pMessage, uiParticipationEndDate);
    _Type_uint32::Add(pMessage, rSource.m_uiMatchmakeSessionCount);
    _Type_uint32::Add(pMessage, rSource.m_uiParticipationCount);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_PersistentGathering::Extract(Message* pMessage, _DDL_PersistentGathering* pDestination)
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
    _Type_uint32::Extract(pMessage, &pDestination->m_uiPersistentGatheringType);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_strPassword);
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_uint32>::Extract(pMessage, &pDestination->m_Attributes);
    if (pMessage->HasError())
        return;
    _Type_qvector<_Type_byte>::Extract(pMessage, &pDestination->m_ApplicationBuffer);
    if (pMessage->HasError())
        return;
    {
        unsigned long long uiParticipationStartDate;
        _Type_uint64::Extract(pMessage, &uiParticipationStartDate);
        pDestination->m_ParticipationStartDate = DateTime(uiParticipationStartDate);
    }
    if (pMessage->HasError())
        return;
    {
        unsigned long long uiParticipationEndDate;
        _Type_uint64::Extract(pMessage, &uiParticipationEndDate);
        pDestination->m_ParticipationEndDate = DateTime(uiParticipationEndDate);
    }
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiMatchmakeSessionCount);
    if (pMessage->HasError())
        return;
    _Type_uint32::Extract(pMessage, &pDestination->m_uiParticipationCount);
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
