#include "nn/nex/_DDL_UserMessage.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/UserMessage.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t USER_MESSAGE_MAX_STRUCTURE_VERSION = 0;

}

_DDL_UserMessage& _DDL_UserMessage::operator=(const _DDL_UserMessage& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiID = rSource.m_uiID;
    m_uiRecipientID = rSource.m_uiRecipientID;
    m_uiRecipientType = rSource.m_uiRecipientType;
    m_uiParentID = rSource.m_uiParentID;
    m_uiSenderID = rSource.m_uiSenderID;
    m_ReceptionTime = rSource.m_ReceptionTime;
    m_uiLifeTime = rSource.m_uiLifeTime;
    m_uiFlags = rSource.m_uiFlags;
    m_Subject = rSource.m_Subject;
    m_SenderName = rSource.m_SenderName;
    return *this;
}

bool _DDL_UserMessage::operator==(const _DDL_UserMessage& rSource) const
{
    return m_uiID == rSource.m_uiID && m_uiRecipientID == rSource.m_uiRecipientID
           && m_uiRecipientType == rSource.m_uiRecipientType && m_uiParentID == rSource.m_uiParentID
           && m_uiSenderID == rSource.m_uiSenderID && m_ReceptionTime == rSource.m_ReceptionTime
           && m_uiLifeTime == rSource.m_uiLifeTime && m_uiFlags == rSource.m_uiFlags
           && String::IsEqual(m_Subject, rSource.m_Subject) && String::IsEqual(m_SenderName, rSource.m_SenderName);
}

void _DDL_UserMessage::Add(Message* pMessage, const _DDL_UserMessage& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = USER_MESSAGE_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiID);
    _Type_uint32::Add(pMessage, rSource.m_uiRecipientID);
    _Type_uint32::Add(pMessage, rSource.m_uiRecipientType);
    _Type_uint32::Add(pMessage, rSource.m_uiParentID);
    _Type_uint32::Add(pMessage, rSource.m_uiSenderID);

    const unsigned long long uiReceptionTime = rSource.m_ReceptionTime;
    _Type_uint64::Add(pMessage, uiReceptionTime);

    _Type_uint32::Add(pMessage, rSource.m_uiLifeTime);
    _Type_uint32::Add(pMessage, rSource.m_uiFlags);
    _Type_string::Add(pMessage, rSource.m_Subject);
    _Type_string::Add(pMessage, rSource.m_SenderName);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        const uint32_t uiSize = uiEndPosition - uiSizePosition - sizeof(uint32_t);
        _Type_uint32::Add(pMessage, uiSize);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_UserMessage::Extract(Message* pMessage, _DDL_UserMessage* pDestination)
{
    _DDL_Data::Extract(pMessage, pDestination);

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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiID);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiRecipientID);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiRecipientType);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiParentID);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiSenderID);

    if (pMessage->HasError())
        return;

    unsigned long long uiReceptionTime;
    _Type_uint64::Extract(pMessage, &uiReceptionTime);
    pDestination->m_ReceptionTime = DateTime(uiReceptionTime);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiLifeTime);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiFlags);

    if (pMessage->HasError())
        return;

    _Type_string::Extract(pMessage, &pDestination->m_Subject);

    if (pMessage->HasError())
        return;

    _Type_string::Extract(pMessage, &pDestination->m_SenderName);

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

Data* _DDL_UserMessage::Clone() const
{
    _DDL_UserMessage* pData = new UserMessage;
    *pData = *this;
    return pData;
}

String _DDL_UserMessage::GetDataType() const
{
    return String(L"UserMessage");
}

bool _DDL_UserMessage::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"UserMessage");
}

bool _DDL_UserMessage::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"UserMessage") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_UserMessage::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_UserMessage::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_UserMessage::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"UserMessage")))
        return pData;

    return 0;
}

const Data* _DDL_UserMessage::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"UserMessage")))
        return pData;

    return 0;
}

}
}
