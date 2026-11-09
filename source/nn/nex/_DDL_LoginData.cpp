#include "nn/nex/_DDL_LoginData.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/LoginData.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t LOGIN_DATA_MAX_STRUCTURE_VERSION = 0;
const unsigned int SIZE_FIELD_LENGTH = 4;

}

_DDL_LoginData& _DDL_LoginData::operator=(const _DDL_LoginData& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_iValue = rSource.m_iValue;
    m_Name = rSource.m_Name;
    m_uiValue64 = rSource.m_uiValue64;
    m_uiValue = rSource.m_uiValue;
    return *this;
}

Data* _DDL_LoginData::Clone() const
{
    LoginData* pData = new LoginData;
    static_cast<_DDL_LoginData&>(*pData) = *this;
    return pData;
}

String _DDL_LoginData::GetDataType() const
{
    return String(L"LoginData");
}

bool _DDL_LoginData::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"LoginData");
}

bool _DDL_LoginData::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"LoginData") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_LoginData::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_LoginData::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_LoginData::Add(Message* pMessage, const _DDL_LoginData& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = LOGIN_DATA_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    pMessage->Append(reinterpret_cast<const unsigned char*>(&rSource.m_iValue), 1, 1);
    _Type_string::Add(pMessage, rSource.m_Name);
    _Type_uint64::Add(pMessage, rSource.m_uiValue64);
    _Type_uint32::Add(pMessage, rSource.m_uiValue);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        const int32_t iSize = static_cast<int32_t>(uiEndPosition - uiSizePosition - SIZE_FIELD_LENGTH);

        pMessage->Append(reinterpret_cast<const unsigned char*>(&iSize), SIZE_FIELD_LENGTH, 1);
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_LoginData::Extract(Message* pMessage, _DDL_LoginData* pDestination)
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

    pMessage->ExtractPrimitive(&pDestination->m_iValue);

    if (pMessage->HasError())
        return;

    _Type_string::Extract(pMessage, &pDestination->m_Name);

    if (pMessage->HasError())
        return;

    _Type_uint64::Extract(pMessage, &pDestination->m_uiValue64);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiValue);

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

Data* _DDL_LoginData::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"LoginData")))
        return pData;

    return 0;
}

const Data* _DDL_LoginData::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"LoginData")))
        return pData;

    return 0;
}

}
}
