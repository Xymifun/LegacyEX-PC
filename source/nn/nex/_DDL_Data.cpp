#include "nn/nex/_DDL_Data.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t DATA_MAX_STRUCTURE_VERSION = 0;

}

_DDL_Data& _DDL_Data::operator=(const _DDL_Data&)
{
    return *this;
}

bool _DDL_Data::operator==(const _DDL_Data&) const
{
    return true;
}

String _DDL_Data::GetDataType() const
{
    return String(L"Data");
}

bool _DDL_Data::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"Data");
}

bool _DDL_Data::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"Data");
}

void _DDL_Data::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_Data::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_Data::Add(Message* pMessage, const _DDL_Data& rSource)
{
    if (!pMessage->IsCompactFormat())
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = DATA_MAX_STRUCTURE_VERSION;
        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));
        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }
}

void _DDL_Data::Extract(Message* pMessage, _DDL_Data* pDestination)
{
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

Data* _DDL_Data::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"Data")))
        return pData;
    return 0;
}

const Data* _DDL_Data::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"Data")))
        return pData;
    return 0;
}

}
}
