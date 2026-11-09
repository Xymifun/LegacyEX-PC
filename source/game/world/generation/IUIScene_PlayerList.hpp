#ifndef GAME_WORLD_GENERATION_IUISCENE_PLAYERLIST_HPP
#define GAME_WORLD_GENERATION_IUISCENE_PLAYERLIST_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/core/IScoreboardLayout.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class INetworkPlayer;
class UIControl_ButtonList;

class IUIScene_PlayerList
{
public:
    struct PlayerChange
    {
        unsigned char smallId;
        bool added;

        PlayerChange(unsigned char smallId, bool added);
    };

    IUIScene_PlayerList();
    virtual ~IUIScene_PlayerList();

    static int KickPlayerReturned(void* context, int playerIndex, C4JStorage::EMessageResult result);
    static void OnPlayerChanged(void* context, INetworkPlayer* player, bool added);

    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual int GetPlayerCount() = 0;
    virtual PlayerInfo* GetPlayer(int index) = 0;
    virtual bool RemovePlayer(unsigned char smallId) = 0;
    virtual void AddPlayer(unsigned char smallId) = 0;
    virtual UIControl_ButtonList* GetPlayerList() = 0;
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo);
    virtual int GetPad() = 0;

    void tick();
    bool hasAPlayerChanged();
    int FindPlayer(unsigned char smallId);

protected:
    OSFastMutex mCriticalSection;
    std::vector<PlayerChange*> mPlayerChanges;
};

#endif
