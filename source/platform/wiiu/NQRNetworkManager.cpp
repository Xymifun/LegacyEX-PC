#include "platform/wiiu/NQRNetworkManager.hpp"

#include "game/world/generation/PlayerUID.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"

int g_iGameBootInviteUserIndex = 0;
bool g_bSafeToRespondToGameBootInvite = false;

NQRNetworkManager::NQRNetworkManager()
    : m_stateChanges(),
      m_stateChangeLock(),
      m_playerLock(),
      m_localPlayerRequestLock(),
      m_localPlayerRequests()
{
}

void NQRNetworkManager::SafeToRespondToGameBootInvite(int iUserIndex)
{
    g_bSafeToRespondToGameBootInvite = true;
    g_iGameBootInviteUserIndex = iUserIndex;
}

int NQRNetworkManager::GetOutstandingAckCount(NQRNetworkPlayer* pPlayer)
{
    int iOutstandingAcks = 0;
    int iPlayerCount = GetPlayerCount();

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pOtherPlayer = GetPlayerByIndex(i);

        if (pOtherPlayer != NULL && (pPlayer == pOtherPlayer || pPlayer->IsSameSystem(pOtherPlayer)))
        {
            iOutstandingAcks += pOtherPlayer->GetTotalOutstandingAckCount();
        }
    }

    return iOutstandingAcks;
}

int NQRNetworkManager::GetSendQueueSizeBytes()
{
    int iSendQueueBytes = 0;
    int iPlayerCount = GetPlayerCount();

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pPlayer = GetPlayerByIndex(i);

        if (pPlayer != NULL)
        {
            iSendQueueBytes += pPlayer->GetTotalSendQueueBytes();
        }
    }

    return iSendQueueBytes;
}

int NQRNetworkManager::GetSendQueueSizeMessages()
{
    int iSendQueueMessages = 0;
    int iPlayerCount = GetPlayerCount();

    for (int i = 0; i < iPlayerCount; i++)
    {
        NQRNetworkPlayer* pPlayer = GetPlayerByIndex(i);

        if (pPlayer != NULL)
        {
            iSendQueueMessages += pPlayer->GetTotalSendQueueMessages();
        }
    }

    return iSendQueueMessages;
}

NQRNetworkManager::PlayerSyncData::PlayerSyncData() : PlayerUID(0)
{
}
