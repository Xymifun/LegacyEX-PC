#include "nn/nex/_DDL_AccountData.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t ACCOUNT_DATA_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_AccountData& _DDL_AccountData::operator=(const _DDL_AccountData& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiPid = rSource.m_uiPid;
    m_Name = rSource.m_Name;
    m_uiGroups = rSource.m_uiGroups;
    m_Email = rSource.m_Email;
    m_Date = rSource.m_Date;
    m_EffectiveDate = rSource.m_EffectiveDate;
    m_NotEffectiveMessage = rSource.m_NotEffectiveMessage;
    m_ExpiryDate = rSource.m_ExpiryDate;
    m_ExpiredMessage = rSource.m_ExpiredMessage;
    return *this;
}

bool _DDL_AccountData::operator==(const _DDL_AccountData& rSource) const
{
    return m_uiPid == rSource.m_uiPid && String::IsEqual(m_Name, rSource.m_Name) && m_uiGroups == rSource.m_uiGroups
           && String::IsEqual(m_Email, rSource.m_Email) && m_Date == rSource.m_Date
           && m_EffectiveDate == rSource.m_EffectiveDate
           && String::IsEqual(m_NotEffectiveMessage, rSource.m_NotEffectiveMessage)
           && m_ExpiryDate == rSource.m_ExpiryDate && String::IsEqual(m_ExpiredMessage, rSource.m_ExpiredMessage);
}

void _DDL_AccountData::Add(Message* pMessage, const _DDL_AccountData& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();
    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = ACCOUNT_DATA_MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();
        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiPid), 4, 1);
    _Type_string::Add(pMessage, rSource.m_Name);
    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_uiGroups), 4, 1);
    _Type_string::Add(pMessage, rSource.m_Email);
    const uint64_t uiDate = rSource.m_Date;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiDate), 8, 1);
    const uint64_t uiEffectiveDate = rSource.m_EffectiveDate;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiEffectiveDate), 8, 1);
    _Type_string::Add(pMessage, rSource.m_NotEffectiveMessage);
    const uint64_t uiExpiryDate = rSource.m_ExpiryDate;
    pMessage->Append(reinterpret_cast<const unsigned char*>(&uiExpiryDate), 8, 1);
    _Type_string::Add(pMessage, rSource.m_ExpiredMessage);
    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);
        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_AccountData::Extract(Message* pMessage, _DDL_AccountData* pDestination)
{
    pDestination->m_uiStructureVersion = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();
    uint32_t uiSize = 0;
    if (!bCompactFormat)
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);
        if (pMessage->HasError())
            return;
        pMessage->ExtractPrimitive(&uiSize);
        if (pMessage->HasError())
            return;
    }
    const unsigned int uiStartPosition = pMessage->GetPosition();
    if (pMessage->HasError())
        return;
    pMessage->ExtractPrimitive(&pDestination->m_uiPid);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_Name);
    if (pMessage->HasError())
        return;
    pMessage->ExtractPrimitive(&pDestination->m_uiGroups);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_Email);
    if (pMessage->HasError())
        return;
    uint64_t uiDate;
    pMessage->ExtractPrimitive(&uiDate);
    pDestination->m_Date = DateTime(uiDate);
    if (pMessage->HasError())
        return;
    uint64_t uiEffectiveDate;
    pMessage->ExtractPrimitive(&uiEffectiveDate);
    pDestination->m_EffectiveDate = DateTime(uiEffectiveDate);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_NotEffectiveMessage);
    if (pMessage->HasError())
        return;
    uint64_t uiExpiryDate;
    pMessage->ExtractPrimitive(&uiExpiryDate);
    pDestination->m_ExpiryDate = DateTime(uiExpiryDate);
    if (pMessage->HasError())
        return;
    _Type_string::Extract(pMessage, &pDestination->m_ExpiredMessage);
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
