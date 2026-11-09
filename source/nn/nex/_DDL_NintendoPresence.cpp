#include "nn/nex/_DDL_NintendoPresence.hpp"

#include <algorithm>

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/NintendoPresence.hpp"
#include "nn/nex/_DDL_GameKey.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t NINTENDO_PRESENCE_MAX_STRUCTURE_VERSION = 0;

}

_DDL_NintendoPresence& _DDL_NintendoPresence::operator=(const _DDL_NintendoPresence& rSource)
{
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    m_uiChangedFlags = rSource.m_uiChangedFlags;
    m_GameKey = rSource.m_GameKey;
    m_strGameModeDescription = rSource.m_strGameModeDescription;
    m_uiJoinAvailabilityFlag = rSource.m_uiJoinAvailabilityFlag;
    m_uiMatchmakeSystemType = rSource.m_uiMatchmakeSystemType;
    m_uiJoinGameID = rSource.m_uiJoinGameID;
    m_uiJoinGameMode = rSource.m_uiJoinGameMode;
    m_uiOwnerPrincipalID = rSource.m_uiOwnerPrincipalID;
    m_uiJoinGroupID = rSource.m_uiJoinGroupID;
    m_ApplicationArg = rSource.m_ApplicationArg;
    return *this;
}

void _DDL_NintendoPresence::Add(Message* pMessage, const _DDL_NintendoPresence& rSource)
{
    _DDL_Data::Add(pMessage, rSource);

    unsigned int uiSizePosition = 0;
    const bool bCompactFormat = pMessage->IsCompactFormat();

    if (!bCompactFormat)
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = NINTENDO_PRESENCE_MAX_STRUCTURE_VERSION;

        _Type_byte::Add(pMessage, std::min(uiVersion, uiMaxVersion));

        uiSizePosition = pMessage->GetLength();

        const uint32_t uiSize = 0;
        _Type_uint32::Add(pMessage, uiSize);
    }

    _Type_uint32::Add(pMessage, rSource.m_uiChangedFlags);
    _DDL_GameKey::Add(pMessage, rSource.m_GameKey);
    _Type_string::Add(pMessage, rSource.m_strGameModeDescription);
    _Type_uint32::Add(pMessage, rSource.m_uiJoinAvailabilityFlag);
    _Type_byte::Add(pMessage, rSource.m_uiMatchmakeSystemType);
    _Type_uint32::Add(pMessage, rSource.m_uiJoinGameID);
    _Type_uint32::Add(pMessage, rSource.m_uiJoinGameMode);
    _Type_uint32::Add(pMessage, rSource.m_uiOwnerPrincipalID);
    _Type_uint32::Add(pMessage, rSource.m_uiJoinGroupID);
    _Type_qvector<_Type_byte>::Add(pMessage, rSource.m_ApplicationArg);

    if (!bCompactFormat)
    {
        const unsigned int uiEndPosition = pMessage->GetLength();

        pMessage->SetLength(uiSizePosition);

        _Type_uint32::Add(pMessage, uiEndPosition - uiSizePosition - sizeof(uint32_t));
        pMessage->SetLength(uiEndPosition);
    }
}

void _DDL_NintendoPresence::Extract(Message* pMessage, _DDL_NintendoPresence* pDestination)
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

    _Type_uint32::Extract(pMessage, &pDestination->m_uiChangedFlags);

    if (pMessage->HasError())
        return;

    _DDL_GameKey::Extract(pMessage, &pDestination->m_GameKey);

    if (pMessage->HasError())
        return;

    _Type_string::Extract(pMessage, &pDestination->m_strGameModeDescription);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiJoinAvailabilityFlag);

    if (pMessage->HasError())
        return;

    _Type_byte::Extract(pMessage, &pDestination->m_uiMatchmakeSystemType);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiJoinGameID);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiJoinGameMode);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiOwnerPrincipalID);

    if (pMessage->HasError())
        return;

    _Type_uint32::Extract(pMessage, &pDestination->m_uiJoinGroupID);

    if (pMessage->HasError())
        return;

    _Type_qvector<_Type_byte>::Extract(pMessage, &pDestination->m_ApplicationArg);

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

Data* _DDL_NintendoPresence::Clone() const
{
    NintendoPresence* pData = new NintendoPresence;
    static_cast<_DDL_NintendoPresence&>(*pData) = *this;
    return pData;
}

String _DDL_NintendoPresence::GetDataType() const
{
    return String(L"NintendoPresence");
}

bool _DDL_NintendoPresence::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoPresence");
}

bool _DDL_NintendoPresence::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"NintendoPresence") || _DDL_Data::IsAKindOf(rType);
}

void _DDL_NintendoPresence::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_NintendoPresence::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

Data* _DDL_NintendoPresence::DynamicCast(Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoPresence")))
        return pData;

    return 0;
}

const Data* _DDL_NintendoPresence::DynamicCast(const Data* pData)
{
    if (pData != 0 && pData->IsAKindOf(String(L"NintendoPresence")))
        return pData;

    return 0;
}

}
}
