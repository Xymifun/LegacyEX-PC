#ifndef GAME_UI_UISCENE_MAINMENU_HPP
#define GAME_UI_UISCENE_MAINMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UILayer.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UILayer;
struct fuiRect;

class UIScene_MainMenu : public UIScene
{
public:
    UIScene_MainMenu(int sceneType, void* sceneData, UILayer* layer);
    virtual ~UIScene_MainMenu();

    virtual unsigned int mapElementsAndNames();
    static void proceedToScene(int playerId, EUIScene scene);
    virtual void handleReload();
    virtual void updateTooltips();
    virtual void updateComponents();
    void customDrawSplash(fuiRect* rect);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual std::wstring getMoviePath();
    virtual void tick();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleUnlockFullVersion();
    void PrepareToPlayGame(int playerId);
    void RunPlayGameOffline(int playerId);
    void RunLeaderboards(int playerId);
    static int Leaderboards_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    void RunUnlockOrDLC(int playerId);
    static int UnlockFullGame_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    static int MustConnectReturnedNintendo(void* context, int pad, C4JStorage::EMessageResult result);
    static int MiniGamesUnlock_Returned(void* context, int playerId, C4JStorage::EMessageResult result);
    static void LoadTrial();
    void RunPlayGame(int playerId);
    static int CreateLoad_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    void RunAchievements(int playerId);
    static int Achievements_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    void RunHelpAndOptions(int playerId);
    static int HelpAndOptions_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    static int ExitGameReturned(void* context, int pad, C4JStorage::EMessageResult result);
    void RunAction(int playerId);
    static int MustSignInReturned(void* context, int pad, C4JStorage::EMessageResult result);
    virtual void handlePress(int controlId, int childId);
    virtual void handleGainFocus(bool hadFocus);
    virtual bool hasFocus(int sceneType);
    virtual int getSceneType();
    virtual unsigned long long getDefaultGtcButtons();

private:
    UIControl_Button m_buttons[8];
    UIControl m_timer;
    bool m_loading;
    bool m_showUpsell;
    bool m_signedIn;
    bool m_miniGamesSelected;
    bool m_trial;
    float m_splashX;
    float m_splashY;
    float m_splashWidth;
    float m_splashHeight;
    std::vector<std::wstring> m_splashes;
    std::wstring m_splash;
    int m_pendingAction;
};

#endif
