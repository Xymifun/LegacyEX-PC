#include "platform/wiiu/NQRNetworkManager_Lan_WiiU.hpp"
#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"

#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/pc/PcVoice.hpp"

bool NQRNetworkManager_WiiU::s_applicationConnected = true;
bool NQRNetworkManager_WiiU::s_piaShutdownPending = false;
bool NQRNetworkManager_Lan_WiiU::s_shutdownTickPending = false;

bool g_bGameBootInvitePending = false;
bool g_bHandlingGameBootInvite = false;
unsigned int g_uiGameBootInvitePrincipalId = 0;

NQRNetworkManager_WiiU::NQRNetworkManager_WiiU(INQRNetworkManagerListener* listener) : PcNetworkSession(listener, true)
{
}

void NQRNetworkManager_WiiU::Pia_ShutdownForLan()
{
}

bool NQRNetworkManager_WiiU::IsHandlingBootInvite()
{
    return g_bHandlingGameBootInvite;
}

void NQRNetworkManager_WiiU::ClearIsHandlingBootInvite()
{
    g_bHandlingGameBootInvite = false;
}

void NQRNetworkManager_WiiU::SetAppSuspended(bool)
{
}

void NQRNetworkManager_WiiU::ShowErrorViewer()
{
}

int NQRNetworkManager_WiiU::ShowConnectOnlineNintendoProgress(int, int (*callback)(void*, bool, int, int),
                                                              void* callbackData)
{
    if (callback != 0)
        callback(callbackData, true, 0, 0);
    return 0;
}

NQRNetworkManager_Lan_WiiU::NQRNetworkManager_Lan_WiiU(INQRNetworkManagerListener* listener)
    : PcNetworkSession(listener), m_saveTransferState(eSaveTransfer_Idle), m_saveTransferOffset(0),
      m_saveTransferSize(0)
{
}

void NQRNetworkManager_Lan_WiiU::SignalInitialiseAfterOnline()
{
}

void NQRNetworkManager_Lan_WiiU::Pia_ShutdownForOnline()
{
}

void NQRNetworkManager_Lan_WiiU::CreateSessionForSaveTransfer(int, void*, int, void*, long long, unsigned int,
                                                              unsigned int, wchar_t*)
{
    m_saveTransferState = eSaveTransfer_CreateSessionFailed;
}

void NintendoVoiceChat_WiiU::startConference()
{
}

void NintendoVoiceChat_WiiU::AddPlayer(unsigned long long)
{
}

void NintendoVoiceChat_WiiU::stopConference()
{
}

void NintendoVoiceChat_WiiU::RemovePlayer(unsigned long long)
{
}

void NintendoVoiceChat_WiiU::RemovePlayer(NQRNetworkPlayer*)
{
}

void NintendoVoiceChat_WiiU::RemoveAllPlayers()
{
}

void NintendoVoiceChat_WiiU::FlushMessageQueue()
{
}

void NintendoVoiceChat_WiiU::Initialise(NQRNetworkManager_WiiU*)
{
}

void NintendoVoiceChat_WiiU::InitialiseAfterOnline()
{
}

void NintendoVoiceChat_WiiU::Shutdown()
{
}

void NintendoVoiceChat_WiiU::AddPlayer(NQRNetworkPlayer*)
{
}

void NintendoVoiceChat_WiiU::SetVoiceChatEnabled(bool)
{
}

bool NintendoVoiceChat_WiiU::IsInConference(NQRNetworkPlayer*)
{
    return false;
}

bool NintendoVoiceChat_WiiU::IsTalking(NQRNetworkPlayer* player)
{
    if (player == 0)
        return false;
    if (player->IsLocal())
        return PcVoiceLocalTalking();
    return PcVoiceSpeakerTalking(player->GetSmallId());
}

void NintendoVoiceChat_WiiU::Tick()
{
}

void NintendoVoiceChat_WiiU::ReleaseCallback()
{
}

void NintendoVoiceChat_WiiU::ReleaseForeground()
{
}

void NintendoVoiceChat_WiiU::AcquiredForeground()
{
}
