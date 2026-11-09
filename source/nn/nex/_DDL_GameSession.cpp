#include "nn/nex/_DDL_GameSession.hpp"

#include "nn/nex/GameSession.hpp"
#include "nn/nex/Message.hpp"

namespace nn
{
namespace nex
{

namespace
{

const uint8_t GAMESESSION_MAX_STRUCTURE_VERSION = 0;

}

_DDL_GameSession& _DDL_GameSession::operator=(const _DDL_GameSession& rSource)
{
    Gathering::operator=(rSource);
    m_uiStructureVersion = rSource.m_uiStructureVersion;
    return *this;
}

void _DDL_GameSession::Add(Message* pMessage, const _DDL_GameSession& rSource)
{
    _DDL_Gathering::Add(pMessage, rSource);
    if (!pMessage->IsCompactFormat())
    {
        uint8_t uiVersion = rSource.m_uiStructureVersion;
        uint8_t uiMaxVersion = GAMESESSION_MAX_STRUCTURE_VERSION;
        pMessage->Append(uiVersion <= uiMaxVersion ? &uiVersion : &uiMaxVersion, 1, 1);
        uint32_t uiSize = 0;
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiSize), sizeof(uiSize), 1);
    }
}

void _DDL_GameSession::Extract(Message* pMessage, _DDL_GameSession* pDestination)
{
    _DDL_Gathering::Extract(pMessage, pDestination);
    if (pMessage->HasError())
        return;
    pDestination->m_uiStructureVersion = 0;
    uint32_t uiSize = 0;
    if (!pMessage->IsCompactFormat())
    {
        pMessage->ExtractPrimitive(&pDestination->m_uiStructureVersion);
        if (!pMessage->HasError())
            pMessage->ExtractPrimitive(&uiSize);
    }
}

Gathering* _DDL_GameSession::Clone() const
{
    return new GameSession;
}

String _DDL_GameSession::GetGatheringType() const
{
    return String(L"GameSession");
}

bool _DDL_GameSession::IsA(const String& rType) const
{
    return String::IsEqual(rType, L"GameSession");
}

bool _DDL_GameSession::IsAKindOf(const String& rType) const
{
    return String::IsEqual(rType, L"GameSession") || _DDL_Gathering::IsAKindOf(rType);
}

void _DDL_GameSession::StreamIn(Message* pMessage) const
{
    Add(pMessage, *this);
}

void _DDL_GameSession::StreamOut(Message* pMessage)
{
    Extract(pMessage, this);
}

}
}
