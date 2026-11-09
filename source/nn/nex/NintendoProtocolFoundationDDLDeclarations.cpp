#include "nn/nex/NintendoProtocolFoundationDDLDeclarations.hpp"

#include <algorithm>

#include "nn/nex/DDLClassMetaType.hpp"
#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/_Type_qlist.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION = 0;

}

_DDL_u8KeyValue& _DDL_u8KeyValue::operator=(const _DDL_u8KeyValue& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiKey = rSource.m_uiKey;
    m_uiValue = rSource.m_uiValue;
    return *this;
}

void _DDL_u8KeyValue::Add(Message* pMessage, const _DDL_u8KeyValue& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_byte::Add(pMessage, rSource.m_uiKey);
    _Type_byte::Add(pMessage, rSource.m_uiValue);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_u8KeyValue::Extract(Message* pMessage, _DDL_u8KeyValue* pDestination)
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

    _Type_byte::Extract(pMessage, &pDestination->m_uiKey);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiValue);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_u8KeyValue::Clone() const
{
    u8KeyValue* pData = new u8KeyValue;
    static_cast<_DDL_u8KeyValue&>(*pData) = *this;
    return pData;
}

String _DDL_u8KeyValue::GetDataType() const
{
    return String(L"u8KeyValue");
}

bool _DDL_u8KeyValue::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"u8KeyValue");
}

bool _DDL_u8KeyValue::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"u8KeyValue") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_u8KeyValue::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_u8KeyValue::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_u8KeyValue::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"u8KeyValue")))
        return pData;

    return 0;
}

const Data* _DDL_u8KeyValue::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"u8KeyValue")))
        return pData;

    return 0;
}

_DDL_u32KeyValue& _DDL_u32KeyValue::operator=(const _DDL_u32KeyValue& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiKey = rSource.m_uiKey;
    m_uiValue = rSource.m_uiValue;
    return *this;
}

void _DDL_u32KeyValue::Add(Message* pMessage, const _DDL_u32KeyValue& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_byte::Add(pMessage, rSource.m_uiKey);
    _Type_uint32::Add(pMessage, rSource.m_uiValue);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_u32KeyValue::Extract(Message* pMessage, _DDL_u32KeyValue* pDestination)
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

    _Type_byte::Extract(pMessage, &pDestination->m_uiKey);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_u32KeyValue::Clone() const
{
    u32KeyValue* pData = new u32KeyValue;
    static_cast<_DDL_u32KeyValue&>(*pData) = *this;
    return pData;
}

String _DDL_u32KeyValue::GetDataType() const
{
    return String(L"u32KeyValue");
}

bool _DDL_u32KeyValue::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"u32KeyValue");
}

bool _DDL_u32KeyValue::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"u32KeyValue") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_u32KeyValue::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_u32KeyValue::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_u32KeyValue::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"u32KeyValue")))
        return pData;

    return 0;
}

const Data* _DDL_u32KeyValue::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"u32KeyValue")))
        return pData;

    return 0;
}

_DDL_u64KeyValue& _DDL_u64KeyValue::operator=(const _DDL_u64KeyValue& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiKey = rSource.m_uiKey;
    m_uiValue = rSource.m_uiValue;
    return *this;
}

void _DDL_u64KeyValue::Add(Message* pMessage, const _DDL_u64KeyValue& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_byte::Add(pMessage, rSource.m_uiKey);
    _Type_uint64::Add(pMessage, rSource.m_uiValue);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_u64KeyValue::Extract(Message* pMessage, _DDL_u64KeyValue* pDestination)
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

    _Type_byte::Extract(pMessage, &pDestination->m_uiKey);

    if (pMessage->HasError())
        return;

    _Type_uint64::Extract(pMessage, &pDestination->m_uiValue);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_u64KeyValue::Clone() const
{
    u64KeyValue* pData = new u64KeyValue;
    static_cast<_DDL_u64KeyValue&>(*pData) = *this;
    return pData;
}

String _DDL_u64KeyValue::GetDataType() const
{
    return String(L"u64KeyValue");
}

bool _DDL_u64KeyValue::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"u64KeyValue");
}

bool _DDL_u64KeyValue::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"u64KeyValue") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_u64KeyValue::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_u64KeyValue::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_u64KeyValue::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"u64KeyValue")))
        return pData;

    return 0;
}

const Data* _DDL_u64KeyValue::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"u64KeyValue")))
        return pData;

    return 0;
}

_DDL_StringKeyValue& _DDL_StringKeyValue::operator=(const _DDL_StringKeyValue& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiKey = rSource.m_uiKey;
    m_strValue = rSource.m_strValue;
    return *this;
}

void _DDL_StringKeyValue::Add(Message* pMessage, const _DDL_StringKeyValue& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_byte::Add(pMessage, rSource.m_uiKey);
    _Type_string::Add(pMessage, rSource.m_strValue);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_StringKeyValue::Extract(Message* pMessage, _DDL_StringKeyValue* pDestination)
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

    _Type_byte::Extract(pMessage, &pDestination->m_uiKey);

    if (pMessage->HasError())
        return;

    _Type_string::Extract(pMessage, &pDestination->m_strValue);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_StringKeyValue::Clone() const
{
    StringKeyValue* pData = new StringKeyValue;
    static_cast<_DDL_StringKeyValue&>(*pData) = *this;
    return pData;
}

String _DDL_StringKeyValue::GetDataType() const
{
    return String(L"StringKeyValue");
}

bool _DDL_StringKeyValue::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"StringKeyValue");
}

bool _DDL_StringKeyValue::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"StringKeyValue") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_StringKeyValue::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_StringKeyValue::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_StringKeyValue::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"StringKeyValue")))
        return pData;

    return 0;
}

const Data* _DDL_StringKeyValue::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"StringKeyValue")))
        return pData;

    return 0;
}

_DDL_NintendoNotificationEventGeneral& _DDL_NintendoNotificationEventGeneral::operator=(
    const _DDL_NintendoNotificationEventGeneral& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiU32Param = rSource.m_uiU32Param;
    m_uiU64Param1 = rSource.m_uiU64Param1;
    m_uiU64Param2 = rSource.m_uiU64Param2;
    m_strStrParam = rSource.m_strStrParam;
    return *this;
}

void _DDL_NintendoNotificationEventGeneral::Add(Message* pMessage, const _DDL_NintendoNotificationEventGeneral& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiU32Param);
    _Type_uint64::Add(pMessage, rSource.m_uiU64Param1);
    _Type_uint64::Add(pMessage, rSource.m_uiU64Param2);
    _Type_string::Add(pMessage, rSource.m_strStrParam);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_NintendoNotificationEventGeneral::Extract(Message* pMessage,
                                                    _DDL_NintendoNotificationEventGeneral* pDestination)
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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiU32Param);

    if (pMessage->HasError())
        return;

    _Type_uint64::Extract(pMessage, &pDestination->m_uiU64Param1);

    if (pMessage->HasError())
        return;

    _Type_uint64::Extract(pMessage, &pDestination->m_uiU64Param2);

    if (pMessage->HasError())
        return;

    _Type_string::Extract(pMessage, &pDestination->m_strStrParam);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_NintendoNotificationEventGeneral::Clone() const
{
    NintendoNotificationEventGeneral* pData = new NintendoNotificationEventGeneral;
    static_cast<_DDL_NintendoNotificationEventGeneral&>(*pData) = *this;
    return pData;
}

String _DDL_NintendoNotificationEventGeneral::GetDataType() const
{
    return String(L"NintendoNotificationEventGeneral");
}

bool _DDL_NintendoNotificationEventGeneral::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoNotificationEventGeneral");
}

bool _DDL_NintendoNotificationEventGeneral::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoNotificationEventGeneral") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_NintendoNotificationEventGeneral::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_NintendoNotificationEventGeneral::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_NintendoNotificationEventGeneral::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoNotificationEventGeneral")))
        return pData;

    return 0;
}

const Data* _DDL_NintendoNotificationEventGeneral::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoNotificationEventGeneral")))
        return pData;

    return 0;
}

_DDL_NintendoNotificationEventProfile& _DDL_NintendoNotificationEventProfile::operator=(
    const _DDL_NintendoNotificationEventProfile& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiRegion = rSource.m_uiRegion;
    m_uiCountry = rSource.m_uiCountry;
    m_uiArea = rSource.m_uiArea;
    m_uiLanguage = rSource.m_uiLanguage;
    m_uiPlatform = rSource.m_uiPlatform;
    return *this;
}

void _DDL_NintendoNotificationEventProfile::Add(Message* pMessage, const _DDL_NintendoNotificationEventProfile& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_byte::Add(pMessage, rSource.m_uiRegion);
    _Type_byte::Add(pMessage, rSource.m_uiCountry);
    _Type_byte::Add(pMessage, rSource.m_uiArea);
    _Type_byte::Add(pMessage, rSource.m_uiLanguage);
    _Type_byte::Add(pMessage, rSource.m_uiPlatform);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_NintendoNotificationEventProfile::Extract(Message* pMessage,
                                                    _DDL_NintendoNotificationEventProfile* pDestination)
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

    _Type_byte::Extract(pMessage, &pDestination->m_uiRegion);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiCountry);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiArea);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiLanguage);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiPlatform);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_NintendoNotificationEventProfile::Clone() const
{
    NintendoNotificationEventProfile* pData = new NintendoNotificationEventProfile;
    static_cast<_DDL_NintendoNotificationEventProfile&>(*pData) = *this;
    return pData;
}

String _DDL_NintendoNotificationEventProfile::GetDataType() const
{
    return String(L"NintendoNotificationEventProfile");
}

bool _DDL_NintendoNotificationEventProfile::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoNotificationEventProfile");
}

bool _DDL_NintendoNotificationEventProfile::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoNotificationEventProfile") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_NintendoNotificationEventProfile::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_NintendoNotificationEventProfile::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_NintendoNotificationEventProfile::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoNotificationEventProfile")))
        return pData;

    return 0;
}

const Data* _DDL_NintendoNotificationEventProfile::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoNotificationEventProfile")))
        return pData;

    return 0;
}

_DDL_NintendoNotificationEventKeyValue& _DDL_NintendoNotificationEventKeyValue::operator=(
    const _DDL_NintendoNotificationEventKeyValue& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_U8KeyValues = rSource.m_U8KeyValues;
    m_U32KeyValues = rSource.m_U32KeyValues;
    m_U64KeyValues = rSource.m_U64KeyValues;
    m_StringKeyValues = rSource.m_StringKeyValues;
    return *this;
}

void _DDL_NintendoNotificationEventKeyValue::Add(Message* pMessage,
                                                 const _DDL_NintendoNotificationEventKeyValue& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PROTOCOL_FOUNDATION_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_qlist<DDLClassMetaType<u8KeyValue> >::Add(pMessage, rSource.m_U8KeyValues);
    _Type_qlist<DDLClassMetaType<u32KeyValue> >::Add(pMessage, rSource.m_U32KeyValues);
    _Type_qlist<DDLClassMetaType<u64KeyValue> >::Add(pMessage, rSource.m_U64KeyValues);
    _Type_qlist<DDLClassMetaType<StringKeyValue> >::Add(pMessage, rSource.m_StringKeyValues);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_NintendoNotificationEventKeyValue::Extract(Message* pMessage,
                                                     _DDL_NintendoNotificationEventKeyValue* pDestination)
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

    _Type_qlist<DDLClassMetaType<u8KeyValue> >::Extract(pMessage, &pDestination->m_U8KeyValues);

    if (pMessage->HasError())
        return;

    _Type_qlist<DDLClassMetaType<u32KeyValue> >::Extract(pMessage, &pDestination->m_U32KeyValues);

    if (pMessage->HasError())
        return;

    _Type_qlist<DDLClassMetaType<u64KeyValue> >::Extract(pMessage, &pDestination->m_U64KeyValues);

    if (pMessage->HasError())
        return;

    _Type_qlist<DDLClassMetaType<StringKeyValue> >::Extract(pMessage, &pDestination->m_StringKeyValues);

    if (pMessage->HasError() || bCompactFormat)
        return;

    if (pMessage->GetPosition() <= uiStartPosition + uiSize && uiStartPosition + uiSize <= pMessage->GetLength())
    {
        pMessage->SetPosition(uiStartPosition + uiSize);
        return;
    }

    pMessage->SetError();
}

Data* _DDL_NintendoNotificationEventKeyValue::Clone() const
{
    NintendoNotificationEventKeyValue* pData = new NintendoNotificationEventKeyValue;
    static_cast<_DDL_NintendoNotificationEventKeyValue&>(*pData) = *this;
    return pData;
}

String _DDL_NintendoNotificationEventKeyValue::GetDataType() const
{
    return String(L"NintendoNotificationEventKeyValue");
}

bool _DDL_NintendoNotificationEventKeyValue::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoNotificationEventKeyValue");
}

bool _DDL_NintendoNotificationEventKeyValue::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoNotificationEventKeyValue") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_NintendoNotificationEventKeyValue::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_NintendoNotificationEventKeyValue::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_NintendoNotificationEventKeyValue::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoNotificationEventKeyValue")))
        return pData;

    return 0;
}

const Data* _DDL_NintendoNotificationEventKeyValue::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoNotificationEventKeyValue")))
        return pData;

    return 0;
}

NintendoProtocolFoundationDDLDeclarations::NintendoProtocolFoundationDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<NintendoProtocolFoundationDDLDeclarations>
    NintendoProtocolFoundationDDLDeclarations::s_declarations;

void NintendoProtocolFoundationDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
