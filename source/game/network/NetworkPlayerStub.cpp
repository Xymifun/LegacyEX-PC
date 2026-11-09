#include "game/network/NetworkPlayerStub.hpp"

#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <limits.h>

#include "game/core/System.hpp"

NetworkPlayerStub::~NetworkPlayerStub()
{
}

unsigned char NetworkPlayerStub::GetSmallId()
{
    return m_qNetPlayer->GetSmallId();
}

void NetworkPlayerStub::SendData(INetworkPlayer* pTo, const void* pData, int iSize, bool bReliable, bool bRequiresAck)
{
    unsigned char* data = static_cast<unsigned char*>(std::malloc(static_cast<size_t>(iSize) + 1));
    if (bRequiresAck)
    {
        data[0] = 1;
        NetworkPlayerStub* destination = static_cast<NetworkPlayerStub*>(pTo);
        destination->m_acksOutstanding++;
        System::processTimeInMilliSecs();
    }
    else
    {
        data[0] = 0;
    }
    std::memcpy(data + 1, pData, static_cast<size_t>(iSize));
    m_qNetPlayer->SendData(static_cast<NetworkPlayerStub*>(pTo)->m_qNetPlayer, data,
                           static_cast<unsigned int>(iSize + 1), 0);
    std::free(data);
}

bool NetworkPlayerStub::IsSameSystem(INetworkPlayer* pOther)
{
    return m_qNetPlayer->IsSameSystem(static_cast<NetworkPlayerStub*>(pOther)->m_qNetPlayer) != false;
}

int NetworkPlayerStub::GetOutstandingAckCount()
{
    return 0;
}

int NetworkPlayerStub::GetSendQueueSizeBytes(INetworkPlayer* pTo, bool bReliable)
{
    IQNetPlayer* destination = pTo ? static_cast<NetworkPlayerStub*>(pTo)->m_qNetPlayer : NULL;
    return static_cast<int>(m_qNetPlayer->GetSendQueueSize(destination, 0));
}

int NetworkPlayerStub::GetSendQueueSizeMessages(INetworkPlayer* pTo, bool bReliable)
{
    IQNetPlayer* destination = pTo ? static_cast<NetworkPlayerStub*>(pTo)->m_qNetPlayer : NULL;
    return static_cast<int>(m_qNetPlayer->GetSendQueueSize(destination, 0));
}

int NetworkPlayerStub::GetCurrentRtt()
{
    return static_cast<int>(m_qNetPlayer->GetCurrentRtt());
}

bool NetworkPlayerStub::IsHost()
{
    return m_qNetPlayer->IsHost() != false;
}

bool NetworkPlayerStub::IsGuest()
{
    return m_qNetPlayer->IsGuest() != false;
}

bool NetworkPlayerStub::IsLocal()
{
    return m_qNetPlayer->IsLocal() != false;
}

int NetworkPlayerStub::GetSessionIndex()
{
    return static_cast<int>(m_qNetPlayer->GetSessionIndex());
}

bool NetworkPlayerStub::IsTalking()
{
    return m_qNetPlayer->IsTalking() != false;
}

bool NetworkPlayerStub::IsMutedByLocalUser(int iUserIndex)
{
    return m_qNetPlayer->IsMutedByLocalUser(static_cast<unsigned int>(iUserIndex)) != false;
}

bool NetworkPlayerStub::HasVoice()
{
    return m_qNetPlayer->HasVoice() != false;
}

bool NetworkPlayerStub::HasCamera()
{
    return m_qNetPlayer->HasCamera() != false;
}

int NetworkPlayerStub::GetUserIndex()
{
    return m_qNetPlayer->GetUserIndex();
}

void NetworkPlayerStub::SetSocket(Socket* pSocket)
{
    m_socket = pSocket;
}

Socket* NetworkPlayerStub::GetSocket()
{
    return m_socket;
}

const wchar_t* NetworkPlayerStub::GetOnlineName()
{
    return m_qNetPlayer->GetGamertag();
}

std::wstring NetworkPlayerStub::GetDisplayName()
{
    return std::wstring(m_qNetPlayer->GetGamertag());
}

PlayerUID NetworkPlayerStub::GetUID()
{
    return m_qNetPlayer->GetXuid();
}

void NetworkPlayerStub::SetUID(PlayerUID uid)
{
}

void NetworkPlayerStub::SentChunkPacket()
{
    m_lastChunkPacketTime = System::processTimeInMilliSecs();
}

int NetworkPlayerStub::GetTimeSinceLastChunkPacket_ms()
{
    if (m_lastChunkPacketTime == 0)
    {
        return INT_MAX;
    }
    long long now = System::processTimeInMilliSecs();
    return static_cast<int>(now - m_lastChunkPacketTime);
}

bool NetworkPlayerStub::isFake()
{
    return false;
}

IQNetPlayer* NetworkPlayerStub::GetQNetPlayer()
{
    return m_qNetPlayer;
}

void NetworkPlayerStub::DecrementAcksOutstanding()
{
    --m_acksOutstanding;
}

int NetworkPlayerStub::GetSinglePlayerOutstandingAckCount()
{
    return m_acksOutstanding;
}
