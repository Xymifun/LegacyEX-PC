#ifndef GAME_UI_UISCENE_JOINMENU_HPP
#define GAME_UI_UISCENE_JOINMENU_HPP
#include <string>
#include <vector>
#include "game/core/CMinecraftApp.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_ButtonList.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIScene.hpp"
class UILayer;
class UIScene_JoinMenu : public UIScene, public IUIPartial_OnlineRestricted
{
public:
    enum EControls
    {
        eControl_JoinGame,
        eControl_GamePlayers,
        eControl_GameInfo
    };
    UIScene_JoinMenu(int, void*, UILayer*);
    virtual unsigned int GetCustomGameModeID();
    virtual unsigned int mapElementsAndNames();
    static void friendSessionUpdated(bool, void*);
    static void AddDescriptionForOption(std::vector<HtmlString>&, eGameHostOption, int);
    static int ErrorDialogReturned(void*, int, C4JStorage::EMessageResult);
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int, int, bool, bool, bool, bool&);
    virtual void handleFocusChange(int, int);
    static void JoinGame(UIScene_JoinMenu*);
    static int WiiUDRCWarningReturned(void*, int, C4JStorage::EMessageResult);
    static int StartGame_SignInReturned(void* context, bool signedIn, int userIndex);
    void StartSharedLaunchFlow();
    virtual void handlePress(int, int);
    virtual int getSceneType();

private:
    UIControl_Button m_joinButton;
    UIControl_ButtonList m_gamePlayers;
    UIControl_HTMLLabel m_gameInfo;
    FriendSessionInfo* m_friendSessionInfo;
    bool m_matchmaking;
    bool m_joining;
    bool m_initialised;
    bool m_friendSessionUpdated;
    bool m_error;
};
#endif
