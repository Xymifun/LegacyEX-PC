#ifndef GAME_UI_UISCENE_PAUSEMENU_HPP
#define GAME_UI_UISCENE_PAUSEMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UIScene_PauseMenu : public UIScene
{
public:
    UIScene_PauseMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_PauseMenu();
    static int SaveWorldThreadProc(void* context);
    static int NetworkErrorHandledDuringInvite(void* context, int userIndex, C4JStorage::EMessageResult result);
    static void _ExitWorld(void* context);
    static int ExitWorldThreadProc(void* context);
    enum EOpenExitGameDialog
    {
        EOpenExitGameDialog_ExitGame = 0,
        EOpenExitGameDialog_JoinInvite = 1
    };
    static void OpenExitGameDialog(int player, void* callbackData,
                                   int (*callback)(void*, int, C4JStorage::EMessageResult),
                                   int (*saveCallback)(void*, int, C4JStorage::EMessageResult),
                                   EOpenExitGameDialog action);
    static int WarningTrialTexturePackReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int ExitGameDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int ExitGameAndSaveReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int ExitGameDeclineSaveReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int ExitGameSaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int SaveGameDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int EnableAutosaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int DisableAutosaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled();
    static int UnlockFullSaveReturned(void* context, int pad, C4JStorage::EMessageResult result);
    void PerformActionSaveGame();
    static unsigned int SaveGame_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    void Init();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handlePreReload();
    virtual void handleReload();
    virtual std::wstring getMoviePath();
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void ShowScene(bool show);
    virtual void SetIgnoreInput(bool ignore);
    static unsigned int BuyTexturePack_SignInReturned(void* context, bool signedIn, int userIndex, int playerIndex);
    bool IsBanEnabled();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    static unsigned int MustSignInReturnedNintendo(void* context, int pad, C4JStorage::EMessageResult result);

private:
    bool m_unusedFlag;
    bool m_texturePackWarningPending;
    bool m_unusedFlag2;
    int m_lastIndex;
    int m_focusIndex;
    int m_unusedValue;
    UIControl_MultiList m_buttonList;
    bool m_ignoreInput;
};

#endif
