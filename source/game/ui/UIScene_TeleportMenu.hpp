#ifndef GAME_UI_UISCENE_TELEPORTMENU_HPP
#define GAME_UI_UISCENE_TELEPORTMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_PlayerButtonList.hpp"

class INetworkPlayer;
class UILayer;

class UIScene_TeleportMenu : public UIScene
{
public:
    struct _PlayerInfo
    {
        unsigned char smallId;
        char voiceStatus;
        short colour;
        std::wstring name;
    };

    UIScene_TeleportMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_TeleportMenu();

    virtual unsigned int mapElementsAndNames();
    _PlayerInfo* BuildPlayerInfo(INetworkPlayer* player);
    static void OnPlayerChanged(void* context, INetworkPlayer* player, bool removed);
    virtual void updateTooltips();
    virtual void handleReload();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handlePress(int controlId, int childId);
    virtual void handleDestroy();
    virtual int getSceneType();
    virtual unsigned int getSubSceneType() const;

private:
    bool m_splitScreen;
    std::vector<_PlayerInfo*> m_playerInfo;
    UIControl_PlayerButtonList m_playerList;
    UIControl_Label m_title;
};

#endif
