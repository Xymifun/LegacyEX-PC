#ifndef GAME_UI_UISCENE_INGAMEPLAYEROPTIONSMENU_HPP
#define GAME_UI_UISCENE_INGAMEPLAYEROPTIONSMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"

class INetworkPlayer;

struct InGamePlayerOptionsData
{
    int playerId;
    unsigned char smallId;
    unsigned int privileges;
};

class UIScene_InGamePlayerOptionsMenu : public UIScene
{
public:
    enum EPlayerOption
    {
        ePlayerOption_BuildAndMine,
        ePlayerOption_UseDoorsAndSwitches,
        ePlayerOption_OpenContainers,
        ePlayerOption_AttackPlayers,
        ePlayerOption_AttackAnimals,
        ePlayerOption_Moderator,
        ePlayerOption_Teleport,
        ePlayerOption_Fly,
        ePlayerOption_DisableExhaustion,
        ePlayerOption_Invisible,
        ePlayerOption_Kick,
        ePlayerOption_GameMode,
        ePlayerOption_SetSpawn
    };

    UIScene_InGamePlayerOptionsMenu(int sceneType, void* data, UILayer* layer);
    void resetCheatCheckboxes();
    static void OnPlayerChanged(void* context, INetworkPlayer* player, bool added);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleDestroy();
    virtual void handleReload();
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxToggled(int id, bool checked);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);

    bool m_navigatingBack;
    unsigned char m_isHost;
    unsigned char m_smallId;
    unsigned int m_privileges;
    UIControl_Label m_gamertag;
    UIControl_MultiList m_playerOptions;
    unsigned char m_cheatsEnabled;
};

#endif
