#include "nn/nex/_DDL_GameKey.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/GameKey.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t GAME_KEY_MAX_STRUCTURE_VERSION = 0;

}

_DDL_GameKey& _DDL_GameKey::operator=(const _DDL_GameKey& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiTitleID = rSource.m_uiTitleID;
    m_uiTitleVersion = rSource.m_uiTitleVersion;
    return *this;
}

void _DDL_GameKey::Add(Message* pMessage, const _DDL_GameKey& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = GAME_KEY_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint64::Add(pMessage, rSource.m_uiTitleID);
    _Type_uint16::Add(pMessage, rSource.m_uiTitleVersion);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_GameKey::Extract(Message* pMessage, _DDL_GameKey* pDestination)
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

    _Type_uint64::Extract(pMessage, &pDestination->m_uiTitleID);

    if (pMessage->HasError())
        return;

    _Type_uint16::Extract(pMessage, &pDestination->m_uiTitleVersion);

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

Data* _DDL_GameKey::Clone() const
{
    GameKey* pData = new GameKey();
    static_cast<_DDL_GameKey&>(*pData) = *this;
    return pData;
}

String _DDL_GameKey::GetDataType() const
{
    return String(L"GameKey");
}

bool _DDL_GameKey::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"GameKey");
}

bool _DDL_GameKey::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"GameKey") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_GameKey::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_GameKey::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_GameKey::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"GameKey")))
        return pData;

    return 0;
}

const Data* _DDL_GameKey::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"GameKey")))
        return pData;

    return 0;
}

}
}
