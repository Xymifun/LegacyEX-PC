#include "nn/nex/AccountManagementServiceDDL.hpp"

#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t PUBLIC_DATA_MAX_STRUCTURE_VERSION = 0;
const uint8_t PRIVATE_DATA_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_PublicData& _DDL_PublicData::operator=(const _DDL_PublicData& rSource)
{
    _DDL_Data::operator=(rSource);
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    return *this;
}

void _DDL_PublicData::Add(Message* pMessage, const _DDL_PublicData& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    if (!pMessage->IsCompactFormat())
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = PUBLIC_DATA_MAX_STRUCTURE_VERSION;

        pMessage->Append(uiVersion <= uiMaxVersion ? &uiVersion : &uiMaxVersion, 1, 1);

        uint32_t uiSize = 0;

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }
}

void _DDL_PublicData::Extract(Message* pMessage, _DDL_PublicData* pDestination)
{
    _DDL_Data::Extract(pMessage, pDestination);

    if (pMessage->HasError())
        return;

    pDestination->m_uiStructureVersion = 0;

    uint32_t uiSize = 0;

    if (!pMessage->IsCompactFormat())
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);

        if (pMessage->HasError())
            return;

        pMessage->ExtractPrimitive(&uiSize);
    }
}

Data* _DDL_PublicData::Clone() const
{
    PublicData* pData = new PublicData;
    static_cast<_DDL_PublicData&>(*pData) = *this;
    return pData;
}

String _DDL_PublicData::GetDataType() const
{
    return String(L"PublicData");
}

bool _DDL_PublicData::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"PublicData");
}

bool _DDL_PublicData::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"PublicData") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_PublicData::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_PublicData::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_PublicData::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"PublicData")))
        return pData;

    return 0;
}

const Data* _DDL_PublicData::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"PublicData")))
        return pData;

    return 0;
}

_DDL_PrivateData& _DDL_PrivateData::operator=(const _DDL_PrivateData& rSource)
{
    _DDL_Data::operator=(rSource);
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    return *this;
}

void _DDL_PrivateData::Add(Message* pMessage, const _DDL_PrivateData& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    if (!pMessage->IsCompactFormat())
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = PRIVATE_DATA_MAX_STRUCTURE_VERSION;

        pMessage->Append(uiVersion <= uiMaxVersion ? &uiVersion : &uiMaxVersion, 1, 1);

        uint32_t uiSize = 0;

        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), SIZE_FIELD_LENGTH, 1);
    }
}

void _DDL_PrivateData::Extract(Message* pMessage, _DDL_PrivateData* pDestination)
{
    _DDL_Data::Extract(pMessage, pDestination);

    if (pMessage->HasError())
        return;

    pDestination->m_uiStructureVersion = 0;

    uint32_t uiSize = 0;

    if (!pMessage->IsCompactFormat())
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);

        if (pMessage->HasError())
            return;

        pMessage->ExtractPrimitive(&uiSize);
    }
}

Data* _DDL_PrivateData::Clone() const
{
    PrivateData* pData = new PrivateData;
    static_cast<_DDL_PrivateData&>(*pData) = *this;
    return pData;
}

String _DDL_PrivateData::GetDataType() const
{
    return String(L"PrivateData");
}

bool _DDL_PrivateData::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"PrivateData");
}

bool _DDL_PrivateData::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"PrivateData") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_PrivateData::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_PrivateData::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_PrivateData::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"PrivateData")))
        return pData;

    return 0;
}

const Data* _DDL_PrivateData::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"PrivateData")))
        return pData;

    return 0;
}

}
}
