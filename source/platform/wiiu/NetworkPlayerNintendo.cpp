#include "platform/wiiu/NetworkPlayerNintendo.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "game/core/System.hpp"

#include <climits>

INetworkPlayer* NetworkPlayerNintendo_Create(NQRNetworkPlayer* player)
{
    return new NetworkPlayerNintendo(player);
}

NetworkPlayerNintendo::NetworkPlayerNintendo(NQRNetworkPlayer* player)
    : m_player(player),
      m_socket(0),
      m_lastChunkPacketTime(0)
{
}

NetworkPlayerNintendo::~NetworkPlayerNintendo()
{
}

unsigned char NetworkPlayerNintendo::GetSmallId()
{
    return m_player->GetSmallId();
}

void NetworkPlayerNintendo::SendData(INetworkPlayer* to, const void* data, int size, bool reliable, bool requiresAck)
{
    NetworkPlayerNintendo* target = static_cast<NetworkPlayerNintendo*>(to);
    m_player->SendData(target->m_player, data, (unsigned int)size, requiresAck);
}

bool NetworkPlayerNintendo::IsSameSystem(INetworkPlayer* other)
{
    NetworkPlayerNintendo* target = static_cast<NetworkPlayerNintendo*>(other);
    return m_player->IsSameSystem(target->m_player);
}

int NetworkPlayerNintendo::GetOutstandingAckCount()
{
    return m_player->GetOutstandingAckCount();
}

int NetworkPlayerNintendo::GetSinglePlayerOutstandingAckCount()
{
    return 0;
}

void NetworkPlayerNintendo::DecrementAcksOutstanding()
{
}

int NetworkPlayerNintendo::GetSendQueueSizeBytes(INetworkPlayer* to, bool reliable)
{
    return m_player->GetSendQueueSizeBytes();
}

int NetworkPlayerNintendo::GetSendQueueSizeMessages(INetworkPlayer* to, bool reliable)
{
    return m_player->GetSendQueueSizeMessages();
}

int NetworkPlayerNintendo::GetCurrentRtt()
{
    return 0;
}

bool NetworkPlayerNintendo::IsHost()
{
    return m_player->IsHost();
}

bool NetworkPlayerNintendo::IsGuest()
{
    PlayerUID uid = m_player->GetUID();
    return (uid.flags >> 29) & 1;
}

bool NetworkPlayerNintendo::IsLocal()
{
    return m_player->IsLocal();
}

int NetworkPlayerNintendo::GetSessionIndex()
{
    return (int)m_player->GetSessionIndex();
}

bool NetworkPlayerNintendo::IsTalking()
{
    return m_player->IsTalking();
}

bool NetworkPlayerNintendo::IsMutedByLocalUser(int userIndex)
{
    return m_player->IsMutedByLocalUser(userIndex);
}

bool NetworkPlayerNintendo::HasVoice()
{
    return m_player->HasVoice();
}

bool NetworkPlayerNintendo::HasCamera()
{
    return false;
}

int NetworkPlayerNintendo::GetUserIndex()
{
    return (int)m_player->GetLocalPlayerIndex();
}

void NetworkPlayerNintendo::SetSocket(Socket* socket)
{
    m_socket = socket;
}

Socket* NetworkPlayerNintendo::GetSocket()
{
    return m_socket;
}

const wchar_t* NetworkPlayerNintendo::GetOnlineName()
{
    return m_player->GetNetworkNameW();
}

std::wstring NetworkPlayerNintendo::GetMiiName()
{
    return std::wstring(m_player->GetMiiName());
}

std::wstring NetworkPlayerNintendo::GetDisplayName()
{
    if (IsLocal())
        return GetMiiName();
    return std::wstring(GetOnlineName());
}

PlayerUID NetworkPlayerNintendo::GetUID()
{
    return m_player->GetUID();
}

void NetworkPlayerNintendo::SetUID(PlayerUID uid)
{
    m_player->SetUID(uid);
}

void NetworkPlayerNintendo::SentChunkPacket()
{
    m_lastChunkPacketTime = System::processTimeInMilliSecs();
}

int NetworkPlayerNintendo::GetTimeSinceLastChunkPacket_ms()
{
    if (m_lastChunkPacketTime == 0)
        return INT_MAX;
    int64_t now = System::processTimeInMilliSecs();
    return (int)(now - m_lastChunkPacketTime);
}

bool NetworkPlayerNintendo::isFake()
{
    return false;
}

IQNetPlayer* NetworkPlayerNintendo::GetQNetPlayer()
{
    return (IQNetPlayer*)m_player;
}
