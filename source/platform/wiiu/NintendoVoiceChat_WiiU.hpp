#ifndef PLATFORM_WIIU_NINTENDOVOICECHAT_WIIU_HPP
#define PLATFORM_WIIU_NINTENDOVOICECHAT_WIIU_HPP

class NQRNetworkManager_WiiU;
class NQRNetworkPlayer;

class NintendoVoiceChat_WiiU
{
public:
    static void startConference();
    static void AddPlayer(unsigned long long stationId);
    static void stopConference();
    static void RemovePlayer(unsigned long long stationId);
    static void RemovePlayer(NQRNetworkPlayer* player);
    static void RemoveAllPlayers();
    static void FlushMessageQueue();
    static void Initialise(NQRNetworkManager_WiiU* manager);
    static void InitialiseAfterOnline();
    static void Shutdown();
    static void AddPlayer(NQRNetworkPlayer* player);
    static void SetVoiceChatEnabled(bool enabled);
    static bool IsInConference(NQRNetworkPlayer* player);
    static bool IsTalking(NQRNetworkPlayer* player);
    static void Tick();
    static void ReleaseCallback();
    static void ReleaseForeground();
    static void AcquiredForeground();
};

#endif
