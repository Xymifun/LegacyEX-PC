#include "nn/nex/_DDL_AuthenticationInfo.hpp"

#include <algorithm>

#include "nn/nex/AuthenticationInfo.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t AUTHENTICATION_INFO_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_AuthenticationInfo& _DDL_AuthenticationInfo::operator=(const _DDL_AuthenticationInfo& rSource)
{
    _DDL_Data::operator=(rSource);
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_Name = rSource.m_Name;
    m_uiValue1 = rSource.m_uiValue1;
    m_uiValue2 = rSource.m_uiValue2;
    m_uiValue3 = rSource.m_uiValue3;
    return *this;
}

Data* _DDL_AuthenticationInfo::Clone() const
{
    AuthenticationInfo* pData = new AuthenticationInfo;
    static_cast<_DDL_AuthenticationInfo&>(*pData) = *this;
    return pData;
}

String _DDL_AuthenticationInfo::GetDataType() const
{
    return String(L"AuthenticationInfo");
}

bool _DDL_AuthenticationInfo::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"AuthenticationInfo");
}

bool _DDL_AuthenticationInfo::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"AuthenticationInfo") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_AuthenticationInfo::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_AuthenticationInfo::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_AuthenticationInfo::Add(Message* pMessage, const _DDL_AuthenticationInfo& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = AUTHENTICATION_INFO_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_string::Add(pMessage, rSource.m_Name);
    _Type_uint32::Add(pMessage, rSource.m_uiValue1);
    _Type_byte::Add(pMessage, rSource.m_uiValue2);
    _Type_uint32::Add(pMessage, rSource.m_uiValue3);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_AuthenticationInfo::Extract(Message* pMessage, _DDL_AuthenticationInfo* pDestination)
{
    _DDL_Data::Extract(pMessage, pDestination);

    if (pMessage->HasError())
        return;

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

    _Type_string::Extract(pMessage, &pDestination->m_Name);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue1);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiValue2);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue3);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_AuthenticationInfo::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"AuthenticationInfo")))
        return pData;

    return 0;
}

const Data* _DDL_AuthenticationInfo::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"AuthenticationInfo")))
        return pData;

    return 0;
}

}
}
