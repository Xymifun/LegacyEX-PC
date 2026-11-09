#ifndef GAME_UI_UIGROUP_HPP
#define GAME_UI_UIGROUP_HPP

#include "game/ui/UILayer.hpp"

enum EUIGroup
{
    EUIGroup_DRC,
    EUIGroup_Fullscreen,
    EUIGroup_Player1,
    EUIGroup_Player2,
    EUIGroup_Player3,
    EUIGroup_Player4,
    EUIGroup_Count,
    EUIGroup_PlayerDefault
};

enum EUILayer
{
    EUILayer_Debug,
    EUILayer_Tooltips,
    EUILayer_Alert,
    EUILayer_Popup,
    EUILayer_Fullscreen,
    EUILayer_Tutorial,
    EUILayer_Scene,
    EUILayer_HUD,
    EUILayer_Count
};

class UIComponent_PressStartToPlay;
class UIComponent_Tooltips;
class UIComponent_TutorialPopup;
class UIComponent_WiiUDRC;
class UIScene_HUD;

class UIGroup
{
    friend class ConsoleUIController;

public:
    UIGroup(EUIGroup group, int player);
    void getRenderDimensions(int& width, int& height);
    unsigned int GetLayerIndex(UILayer* layer);
    unsigned int RequestFocus(UILayer* layer);
    void UpdateFocusState();
    bool IsFullscreenGroup();
    unsigned int GetViewportType();
    void DestroyAll();
    void ReloadAll();
    void _UpdateFocusState();
    void tick();
    void render();
    int hidesLowerScenes();
    void updateStackStates();
    bool NavigateToScene(int player, EUIScene scene, void* data, EUILayer layer);
    int NavigateBack(int player, EUIScene scene, EUILayer layer);
    void closeAllScenes();
    UIScene* GetTopScene(EUILayer layer);
    bool IsSceneInStack(EUIScene scene);
    bool GetMenuDisplayed();
    void handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled);
    UIScene* getCurrentScene(int player);
    void showComponent(int player, EUIScene scene, EUILayer layer, bool show);
    UIScene* addComponent(int player, EUIScene scene, EUILayer layer);
    void removeComponent(EUIScene scene, EUILayer layer);
    void ReloadAllLayers();
    void SetViewportType(C4JRender::eViewportType viewport);
    void HandleDLCMountingComplete();
    void HandleDLCInstalled();
    void HandleDLCLicenseChange();
    void HandleTMSBanFileRetrieved();
    void HandleMessage(EUIMessage message, void* data);
    void HandleSaveDeviceRemoved(int device);
    UIScene* FindScene(EUIScene scene);
    void ClearTooltips();
    bool HasFocus(int player);
    void handleUnlockFullVersion();
    void* getCommandBufferList();
    UIComponent_Tooltips* getTooltips() { return m_tooltips; }
    UIComponent_TutorialPopup* getTutorialPopup() { return m_tutorialPopup; }
    UIComponent_PressStartToPlay* getPressStartToPlay() { return m_pressStartToPlay; }
    UIScene_HUD* getHUD() { return m_hud; }
    UIComponent_WiiUDRC* getDRC() { return m_drc; }

private:
    UILayer* m_layers[EUILayer_Count];
    UIComponent_Tooltips* m_tooltips;
    UIComponent_TutorialPopup* m_tutorialPopup;
    UIComponent_PressStartToPlay* m_pressStartToPlay;
    UIScene_HUD* m_hud;
    UIComponent_WiiUDRC* m_drc;
    int m_viewportType;
    EUIGroup m_group;
    int m_player;
    bool m_menuDisplayed;
    bool m_pauseMenuDisplayed;
    bool m_containerMenuDisplayed;
    bool m_ignoreAutosaveMenuDisplayed;
    bool m_ignorePlayerJoinMenuDisplayed;
    int m_updateFocusStateCountdown;
    int m_commandBufferList;
};

#endif
