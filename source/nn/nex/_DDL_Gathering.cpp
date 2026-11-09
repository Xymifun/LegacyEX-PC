#include "nn/nex/_DDL_Gathering.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t GATHERING_MAX_STRUCTURE_VERSION = 0;

}

_DDL_Gathering& _DDL_Gathering::operator=(const _DDL_Gathering& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiValue1 = rSource.m_uiValue1;
    m_uiValue2 = rSource.m_uiValue2;
    m_uiValue3 = rSource.m_uiValue3;
    m_uiShortValue1 = rSource.m_uiShortValue1;
    m_uiShortValue2 = rSource.m_uiShortValue2;
    m_uiParticipationPolicy = rSource.m_uiParticipationPolicy;
    m_uiPolicyArgument = rSource.m_uiPolicyArgument;
    m_uiValue4 = rSource.m_uiValue4;
    m_uiValue5 = rSource.m_uiValue5;
    m_Description = rSource.m_Description;
    return *this;
}

String _DDL_Gathering::GetGatheringType() const
{
    return String(L"Gathering");
}

bool _DDL_Gathering::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"Gathering");
}

bool _DDL_Gathering::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"Gathering");
}

void _DDL_Gathering::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_Gathering::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_Gathering::Add(Message* pMessage, const _DDL_Gathering& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = GATHERING_MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiValue1);
    _Type_uint32::Add(pMessage, rSource.m_uiValue2);
    _Type_uint32::Add(pMessage, rSource.m_uiValue3);
    _Type_uint16::Add(pMessage, rSource.m_uiShortValue1);
    _Type_uint16::Add(pMessage, rSource.m_uiShortValue2);
    _Type_uint32::Add(pMessage, rSource.m_uiParticipationPolicy);
    _Type_uint32::Add(pMessage, rSource.m_uiPolicyArgument);
    _Type_uint32::Add(pMessage, rSource.m_uiValue4);
    _Type_uint32::Add(pMessage, rSource.m_uiValue5);
    _Type_string::Add(pMessage, rSource.m_Description);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_Gathering::Extract(Message* pMessage, _DDL_Gathering* pDestination)
{
    pDestination->m_uiStructureVersion = 0;

    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;

    if (!bCompactFormat)
    {
        _Type_byte::Extract(pMessage, &pDestination->m_uiStructureVersion);
        if (pMessage->HasError())
        {
            return;
        }

        _Type_uint32::Extract(pMessage, &uiSize);
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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue1);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue2);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue3);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint16::Extract(pMessage, &pDestination->m_uiShortValue1);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint16::Extract(pMessage, &pDestination->m_uiShortValue2);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiParticipationPolicy);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiPolicyArgument);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue4);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue5);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_string::Extract(pMessage, &pDestination->m_Description);
    if (pMessage->HasError() || bCompactFormat)
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
