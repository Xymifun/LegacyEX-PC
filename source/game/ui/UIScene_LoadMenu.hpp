#ifndef GAME_UI_UISCENE_LOADMENU_HPP
#define GAME_UI_UISCENE_LOADMENU_HPP

#include <stdint.h>
#include <string>

#include "game/core/IUIScene_StartGame.hpp"
#include "game/core/SliderLogic.hpp"
#include "game/minigame/IUIScene_MiniGames.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_MultiList.hpp"

class UILayer;

class UIScene_LoadMenu : public IUIScene_StartGame, public virtual IUIScene_MiniGames
{
public:
    UIScene_LoadMenu(int sceneType, void* data, UILayer* layer);

    virtual void AddVariantButtons(bool alternate);
    void UpdateIfPublicGamesAllowed();
    void UpdateDescriptionText();
    void _initCustom();
    static int LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size);
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual unsigned int getSubSceneType() const;
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxToggled(int id, bool checked);
    virtual void handleCheckboxElementToggled(int first, int second, bool checked);
    virtual void handleSliderElementMove(int first, int second, int value);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleTimerComplete(int timerId);
    void checkDialogReturned(bool& value, C4JStorage::EMessageResult result);
    static uint32_t CheckResetNetherReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    static uint32_t CheckUpdateTheEndReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    virtual void checkStateAndStartGame();
    virtual UIControl* GetMainPanel();
    void SetEnablePlayGameButton(bool enabled);
    virtual void HandleSaveDeviceRemoved(int deviceIndex);
    virtual int getSceneType();

private:
    SliderLogic m_seedLogic;
    SliderLogic m_sizeLogic;
    SliderLogic m_gameModeLogic;
    SliderLogic m_difficultyLogic;
    UIControl m_mainPanel;
    UIControl_Label m_gameName;
    UIControl_Label m_description;
    UIControl_Label m_createdMode;
    UIControl_Button m_playButton;
    UIControl_Button m_loadSettings;
    UIControl_BitmapIcon m_levelIcon;
    UIControl_MultiList m_list;
    bool m_thumbnailPending;
    bool m_thumbnailLoaded;
    bool m_loadRequested;
    bool m_thumbnailStateUnknown;
    bool m_thumbnailFailed;
    bool m_updateInProgress;
    bool m_publicGamesAllowed;
    double m_focus;
    double m_childFocus;
    bool m_resetNetherPending;
    bool m_updateEndPending;
    bool m_hasGameData;
#ifdef MINECRAFT_PC
    int m_availableGames;
    unsigned long long m_availableCheckedAt;
#endif
    unsigned char* m_thumbnailData;
    unsigned int m_thumbnailSize;
    std::wstring m_thumbnailName;
};

#endif
