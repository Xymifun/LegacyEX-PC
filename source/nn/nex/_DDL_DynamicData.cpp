#include "nn/nex/_DDL_DynamicData.hpp"

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DynamicData.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

_DDL_DynamicData& _DDL_DynamicData::operator=(const _DDL_DynamicData& rSource)
{
    m_uiDynamicStructureVersion = rSource.m_uiDynamicStructureVersion;
    m_Buffer = rSource.m_Buffer;
    return *this;
}

Data* _DDL_DynamicData::Clone() const
{
    _DDL_DynamicData* pClone = new DynamicData;
    *pClone = *this;
    return pClone;
}

String _DDL_DynamicData::GetDataType() const
{
    return String(L"DynamicData");
}

bool _DDL_DynamicData::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"DynamicData");
}

bool _DDL_DynamicData::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"DynamicData") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_DynamicData::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_DynamicData::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_DynamicData::Add(Message* pMessage, const _DDL_DynamicData& rSource)
{
    _DDL_Data::Add(pMessage, rSource);
    _Type_buffertail::Add(pMessage, rSource.m_Buffer);
}

void _DDL_DynamicData::Extract(Message* pMessage, _DDL_DynamicData* pDestination)
{
    _DDL_Data::Extract(pMessage, pDestination);
    if (!pMessage->HasError())
    {
        pDestination->m_uiDynamicStructureVersion = 0;
        _Type_buffertail::Extract(pMessage, &pDestination->m_Buffer);
    }
}

Data* _DDL_DynamicData::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"DynamicData")))
        return pData;
    return 0;
}

const Data* _DDL_DynamicData::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"DynamicData")))
        return pData;
    return 0;
}

}
}
