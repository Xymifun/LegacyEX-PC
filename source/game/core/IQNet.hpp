#ifndef GAME_CORE_IQNET_HPP
#define GAME_CORE_IQNET_HPP

#include "game/world/generation/IQNetPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"

enum QNET_STATE
{
    QNET_STATE_IDLE,
    QNET_STATE_SESSION_HOSTING,
    QNET_STATE_SESSION_JOINING,
    QNET_STATE_GAME_LOBBY,
    QNET_STATE_SESSION_REGISTERING,
    QNET_STATE_SESSION_STARTING,
    QNET_STATE_GAME_PLAY,
    QNET_STATE_SESSION_ENDING,
    QNET_STATE_SESSION_LEAVING,
    QNET_STATE_SESSION_DELETING
};

class IQNet
{
public:
    int AddLocalPlayerByUserIndex(unsigned int userIndex);
    IQNetPlayer* GetHostPlayer();
    IQNetPlayer* GetLocalPlayerByUserIndex(unsigned int userIndex);
    IQNetPlayer* GetPlayerByIndex(unsigned int playerIndex);
    IQNetPlayer* GetPlayerBySmallId(unsigned char smallId);
    IQNetPlayer* GetPlayerByXuid(PlayerUID xuid);
    unsigned int GetPlayerCount();
    QNET_STATE GetState();
    bool IsHost();
    void HostGame();
    void EndGame();
};

#endif
