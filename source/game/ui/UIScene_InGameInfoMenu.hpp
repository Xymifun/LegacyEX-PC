#ifndef GAME_UI_UISCENE_INGAMEINFOMENU_HPP
#define GAME_UI_UISCENE_INGAMEINFOMENU_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/world/generation/IUIScene_PlayerList.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_ButtonList.hpp"
#include "game/ui/UIControl_PlayerButtonList.hpp"
#include "game/ui/UIControl_Label.hpp"

class UILayer;
struct PlayerInfo;

class UIScene_InGameInfoMenu : public UIScene, public IUIScene_PlayerList
{
public:
    UIScene_InGameInfoMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_InGameInfoMenu();

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handlePress(int controlId, int childId);
    virtual void handleDestroy();
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void tick();
    virtual void updateTooltips();

    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo);
    virtual int GetPlayerCount();
    virtual PlayerInfo* GetPlayer(int index);
    virtual bool RemovePlayer(unsigned char smallId);
    virtual void AddPlayer(unsigned char smallId);
    virtual UIControl_ButtonList* GetPlayerList();
    virtual int GetPad();
    virtual int getSceneType();

    bool hasSomethingToEdit(unsigned char smallId);

private:
    bool m_isHost;
    std::vector<PlayerInfo*> m_players;
    UIControl_Button m_optionsButton;
    UIControl_PlayerButtonList m_playerList;
    UIControl_Label m_titleLabel;
};

#endif
