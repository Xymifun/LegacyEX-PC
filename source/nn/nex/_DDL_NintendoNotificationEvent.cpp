#include "nn/nex/_DDL_NintendoNotificationEvent.hpp"

#include "nn/nex/DDLClassMetaType.hpp"
#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t NINTENDO_NOTIFICATION_EVENT_MAX_STRUCTURE_VERSION = 0;

typedef _Type_any<DDLClassMetaType<Data>, _Type_string> NotificationDataAny;

}

_DDL_NintendoNotificationEvent& _DDL_NintendoNotificationEvent::operator=(const _DDL_NintendoNotificationEvent& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiType = rSource.m_uiType;
    m_uiValue = rSource.m_uiValue;
    m_Data = rSource.m_Data;
    return *this;
}

void _DDL_NintendoNotificationEvent::Add(Message* pMessage, const _DDL_NintendoNotificationEvent& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_NOTIFICATION_EVENT_MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiType);
    _Type_uint32::Add(pMessage, rSource.m_uiValue);
    NotificationDataAny::Add(pMessage, rSource.m_Data);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_NintendoNotificationEvent::Extract(Message* pMessage, _DDL_NintendoNotificationEvent* pDestination)
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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiType);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue);
    if (pMessage->HasError())
    {
        return;
    }

    NotificationDataAny::Extract(pMessage, &pDestination->m_Data);
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
