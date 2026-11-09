#include "nn/nex/_DDL_NotificationEvent.hpp"

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

_DDL_NotificationEvent& _DDL_NotificationEvent::operator=(const _DDL_NotificationEvent& rSource)
{
    m_uiValue = rSource.m_uiValue;
    m_uiType = rSource.m_uiType;
    m_uiParam1 = rSource.m_uiParam1;
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiParam2 = rSource.m_uiParam2;
    m_StringParam = rSource.m_StringParam;
    m_uiParam3 = rSource.m_uiParam3;
    return *this;
}

void _DDL_NotificationEvent::Add(Message* pMessage, const _DDL_NotificationEvent& rSource)
{
    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = 0;
        pMessage->Append(uiVersion <= uiMaxVersion ? &uiVersion : &uiMaxVersion, 1, 1);
        uiSizePosition = pMessage->GetLength();

        uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiValue);
    _Type_uint32::Add(pMessage, rSource.m_uiType);
    _Type_uint32::Add(pMessage, rSource.m_uiParam1);
    _Type_uint32::Add(pMessage, rSource.m_uiParam2);
    _Type_string::Add(pMessage, rSource.m_StringParam);
    _Type_uint32::Add(pMessage, rSource.m_uiParam3);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();
        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_NotificationEvent::Extract(Message* pMessage, _DDL_NotificationEvent* pDestination)
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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiType);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiParam1);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiParam2);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_string::Extract(pMessage, &pDestination->m_StringParam);
    if (pMessage->HasError())
    {
        return;
    }

    _Type_uint32::Extract(pMessage, &pDestination->m_uiParam3);
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
