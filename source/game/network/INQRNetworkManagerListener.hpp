#ifndef GAME_NETWORK_INQRNETWORKMANAGERLISTENER_HPP
#define GAME_NETWORK_INQRNETWORKMANAGERLISTENER_HPP

#include "platform/wiiu/NQRNetworkManager.hpp"

class NQRNetworkPlayer;

namespace nn
{
namespace fp
{
struct FriendPresence;
}
}

class INQRNetworkManagerListener
{
public:
    virtual ~INQRNetworkManagerListener() {}

    virtual void HandleDataReceived(NQRNetworkPlayer* pFrom, NQRNetworkPlayer* pTo, unsigned char* pData,
                                    unsigned int uiSize) = 0;
    virtual void HandlePlayerJoined(NQRNetworkPlayer* pPlayer) = 0;
    virtual void HandlePlayerLeaving(NQRNetworkPlayer* pPlayer) = 0;
    virtual void HandleStateChange(NQRNetworkManager::eNQRNetworkManagerState eOldState,
                                   NQRNetworkManager::eNQRNetworkManagerState eNewState, bool bError) = 0;
    virtual void HandleResyncPlayerRequest(NQRNetworkPlayer** ppPlayers) = 0;
    virtual void HandleAddLocalPlayerFailed(int iUserIndex) = 0;
    virtual void HandleDisconnect(bool bError) = 0;
    virtual void HandleInviteReceived(int iUserIndex, nn::fp::FriendPresence* pFriendPresence) = 0;
};

#endif
