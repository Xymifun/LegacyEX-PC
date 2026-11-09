#ifndef GAME_UI_UILAYER_HPP
#define GAME_UI_UILAYER_HPP

#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/ui/UIScene.hpp"

class UIGroup;

enum EUIScene
{
    EUIScene_Unknown = 0,
    EUIScene_Intro = 1,
    EUIScene_SaveMessage = 2,
    EUIScene_MainMenu = 3,
    EUIScene_FullscreenProgress = 4,
    EUIScene_PauseMenu = 5,
    EUIScene_CraftingMenu2x2 = 6,
    EUIScene_CraftingMenu = 7,
    EUIScene_FurnaceMenu = 8,
    EUIScene_ContainerMenu = 9,
    EUIScene_ContainerMenuLarge = 0xa,
    EUIScene_InventoryMenu = 0xb,
    EUIScene_DispenserMenu = 0xc,
    EUIScene_HelpAndOptionsMenu = 0xf,
    EUIScene_HowToPlay = 0x10,
    EUIScene_HowToPlayMenu = 0x11,
    EUIScene_ControlsMenu = 0x12,
    EUIScene_SettingsListMenu = 0x13,
    EUIScene_InGameSettingsListMenu = 0x14,
    EUIScene_SettingsMenu = 0x15,
    EUIScene_LeaderboardsMenu = 0x16,
    EUIScene_Credits = 0x17,
    EUIScene_DeathMenu = 0x18,
    EUIScene_TutorialPopup = 0x19,
    EUIScene_CreateWorldMenu = 0x1a,
    EUIScene_LoadCreateJoinMenu = 0x1b,
    EUIScene_JoinMenu = 0x1c,
    EUIScene_SignEntryMenu = 0x1d,
    EUIScene_InGameInfoMenu = 0x1e,
    EUIScene_ConnectingProgress = 0x1f,
    EUIScene_DLCOffersMenu = 0x20,
    EUIScene_TrialExitUpsell = 0x22,
    EUIScene_LoadMenu = 0x23,
    EUIScene_Chat = 0x24,
    EUIScene_ReinstallMenu = 0x25,
    EUIScene_SkinSelectMenu = 0x26,
    EUIScene_InGameHostOptionsMenu = 0x28,
    EUIScene_InGamePlayerOptionsMenu = 0x29,
    EUIScene_CreativeMenu = 0x2a,
    EUIScene_LaunchMoreOptionsMenu = 0x2b,
    EUIScene_DLCMainMenu = 0x2c,
    EUIScene_NewUpdateMessage = 0x2d,
    EUIScene_EnchantingMenu = 0x2e,
    EUIScene_BrewingStandMenu = 0x2f,
    EUIScene_EndPoem = 0x30,
    EUIScene_HUD = 0x31,
    EUIScene_TradingMenu = 0x32,
    EUIScene_AnvilMenu = 0x33,
    EUIScene_TeleportMenu = 0x34,
    EUIScene_HopperMenu = 0x35,
    EUIScene_BeaconMenu = 0x36,
    EUIScene_HorseInventoryMenu = 0x37,
    EUIScene_ClassicCraftingMenu = 0x38,
    EUIScene_SuperFlatMenu = 0x39,
    EUIScene_SuperFlatEditLayer = 0x3a,
    EUIScene_SuperFlatPresets = 0x3b,
    EUIScene_GameModeScoreboardMenu = 0x3c,
    EUIScene_WritableBookMenu = 0x3d,
    EUIScene_MapVoteMenu = 0x3e,
    EUIScene_MapSelectMenu = 0x3f,
    EUIScene_BiomePreviewMenu = 0x40,
    EUIScene_WiiUDRC = 0x41,
    EUIScene_Panorama = 0x42,
    EUIScene_Logo = 0x43,
    EUIScene_DebugUIConsole = 0x44,
    EUIScene_DebugUIMarketingGuide = 0x45,
    EUIScene_Tooltips = 0x46,
    EUIScene_PressStartToPlay = 0x47,
    EUIScene_MenuBackground = 0x48,
    EUIScene_Keyboard = 0x49,
    EUIScene_QuadrantSignin = 0x4a,
    EUIScene_MessageBox = 0x4b,
    EUIScene_Timer = 0x4c,
    EUIScene_EULA = 0x4d,
    EUIScene_InGameSaveManagementMenu = 0x4e,
    EUIScene_LanguageSelector = 0x4f,
    EUIScene_AvatarSelectMenu = 0x50,
    EUIScene_AchievementsMenu = 0x51,
    EUIScene_DebugSetCamera = 0x52,
    EUIScene_DebugFillArea = 0x27,
    EUIScene_DebugCreateSchematic = 0x21,
#ifdef MINECRAFT_PC
    EUIScene_PcGlideSections = 0xd,
#endif
    EUIScene_None = 0x53,
    EUIScene_Count
};

class UILayer
{
public:
    UILayer(UIGroup* group);

    void getRenderDimensions(int& width, int& height);
    UIScene* addComponent(int player, EUIScene scene, void* data);
    void showComponent(int player, EUIScene scene, bool visible);
    void tick();
    void render(int width, int height, C4JRender::eViewportType viewport);
    void DestroyAll();
    void ReloadAll(bool force);
    bool IsSceneInStack(EUIScene scene);
    bool updateFocusState(bool requestFocus);
    void closeAllScenes();
    bool NavigateToScene(int player, EUIScene scene, void* data);
    void removeScene(UIScene* scene);
    bool NavigateBack(int player, EUIScene scene);
    UIScene* GetTopScene();
    bool HasFocus(int controllerId);
    bool hidesLowerScenes();
    bool isComponentVisible(EUIScene scene);
    void removeComponent(EUIScene scene);
    void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    UIScene* getCurrentScene(int controllerId);
    bool IsFullscreenGroup();
    C4JRender::eViewportType getViewport();
    void HandleDLCMountingComplete();
    void HandleDLCInstalled();
    void HandleDLCLicenseChange();
    void HandleTMSBanFileRetrieved();
    void HandleMessage(EUIMessage message, void* data);
    void HandleSaveDeviceRemoved(int device);
    void handleUnlockFullVersion();
    UIScene* FindScene(EUIScene scene);
    void DestroyAllScenes();
    bool GetMenuDisplayed();

private:
    std::vector<UIScene*> m_scenes;
    std::vector<UIScene*> m_components;
    std::vector<UIScene*> m_pendingDestroy;
    std::vector<UIScene*> m_pendingReload;
    boost::unordered_map<EUIScene, std::pair<int, bool> > m_componentStates;
    bool m_hasFocus;
    bool m_menuDisplayed;
    bool m_pauseMenuDisplayed;
    bool m_containerMenuDisplayed;
    bool m_ignoreAutosaveMenuDisplayed;
    bool m_ignorePlayerJoinMenuDisplayed;
    int m_layerId;
    UIGroup* m_group;

    friend class ConsoleUIController;
    friend class UIGroup;
    friend class UIScene_FullscreenProgress;
    friend class UIControl_Button;
    friend class UIControl_CheckBox;
    friend class UIControl_DynamicLabel;
    friend class UIControl_HTMLLabel;
    friend class UIControl_LeaderboardList;
    friend class UIControl_List;
    friend class UIControl_Slider;
    friend class UIControl_TextInput;
    friend class UIControl_TexturePackList;
    friend class UIControl_Touch;
    friend class UIControl_GridList;
};

#endif
