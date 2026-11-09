#include "nn/nex/_DDL_DynamicGathering.hpp"

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DynamicGathering.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

Gathering* _DDL_DynamicGathering::Clone() const
{
    return new DynamicGathering;
}

String _DDL_DynamicGathering::GetGatheringType() const
{
    return String(L"DynamicGathering");
}

bool _DDL_DynamicGathering::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"DynamicGathering");
}

bool _DDL_DynamicGathering::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"DynamicGathering") || _DDL_Gathering::IsAKindOf(rType);
}

void _DDL_DynamicGathering::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_DynamicGathering::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

void _DDL_DynamicGathering::Add(Message* pMessage, const _DDL_DynamicGathering& rSource)
{
    _DDL_Gathering::Add(pMessage, rSource);
    _Type_buffertail::Add(pMessage, rSource.m_Buffer);
}

void _DDL_DynamicGathering::Extract(Message* pMessage, _DDL_DynamicGathering* pDestination)
{
    _DDL_Gathering::Extract(pMessage, pDestination);
    if (pMessage->HasError())
    {
        return;
    }

    pDestination->m_uiDynamicStructureVersion = 0;
    if (pMessage->HasError())
    {
        return;
    }

    _Type_buffertail::Extract(pMessage, &pDestination->m_Buffer);
}

}
}
