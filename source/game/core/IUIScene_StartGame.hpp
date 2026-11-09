#ifndef GAME_CORE_IUISCENE_STARTGAME_HPP
#define GAME_CORE_IUISCENE_STARTGAME_HPP

#include <string>
#include <vector>

#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Slider.hpp"
#include "game/ui/UIControl_TexturePackList.hpp"
#include "game/ui/UIScene.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class DLCPack;
class LevelGenerationOptions;
class UILayer;

class IUIScene_StartGame : public UIScene, public virtual IUIPartial_OnlineRestricted
{
public:
    struct TEXTUREPACKSORTSTRUCT
    {
        int sortValue;
        int packId;
    };

    IUIScene_StartGame(int sceneType, void* data, UILayer* layer, bool customGame);
    virtual ~IUIScene_StartGame();

    virtual unsigned int mapElementsAndNames();
    static bool SortTexturePacks(const TEXTUREPACKSORTSTRUCT& first, const TEXTUREPACKSORTSTRUCT& second);
    void UpdateTexturePackDescription(int packId);
    void OrganizeTexturePacks();
    void _init();
    void RequestPrivMessageBox();
    void FindAndJoinGame(unsigned int playerMask);
    void HostGame(unsigned int playerMask);
    void StartGame(unsigned int playerMask);
    static int StartGame_SignInReturned(void* context, bool signedIn, int userIndex);
    virtual void tick();
    virtual void handleReload();
    virtual void handleSelectionChanged(int selection);
    virtual void handleSliderMove(int id, int value);
    virtual void handleTouchBoxRebuild();
    virtual void handleTimerComplete(int timerId);
    virtual void HandleDLCMountingComplete();
    void UpdateCurrentTexturePack(int slot);
    static int TrialTexturePackWarningReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    static int UnlockTexturePackReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    static int TexturePackDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    static int TexturePackDialogBattleMiniGameReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    void CheckPermissionsAndContinue(bool fromLoad);
    static int WiiUDRCWarningReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    void StartSharedLaunchFlow();
    static int DeleteSaveDataReturned(void* context, bool deleted);
    static int DeleteSaveDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    static int LoadDataComplete(void* context);
    static int LoadSaveDataReturned(void* context, bool corrupt, bool complete);
    static int CheckCreativeReturned(void* context, int userIndex, C4JStorage::EMessageResult result);
    void LaunchGame();
    int getCurrentTextureSelectorIndex(int packId);
    static int TrophyDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result);

    virtual void checkStateAndStartGame() = 0;
    virtual unsigned int GetCustomGameModeID();
    virtual void SetEnablePlayGameButton(bool enable) = 0;

protected:
    UIControl_TexturePackList m_texturePackList;
    UIControl m_mainPanel;
    UIControl_Label m_texturePackName;
    UIControl_HTMLLabel m_texturePackDescription;
    UIControl_BitmapIcon m_texturePackIcon;
    UIControl_BitmapIcon m_texturePackComparison;
    UIControl_Slider m_difficultySlider;
    UIControl_Slider m_gameModeSlider;
    UIControl_CheckBox m_onlineGameCheckBox;
    LaunchMoreOptionsMenuInitData m_launchData;
    bool m_actionPending;
    DLCPack* m_texturePackParentPack;
    LevelGenerationOptions* m_levelGenerationOptions;
    std::vector<TEXTUREPACKSORTSTRUCT> m_texturePacks;
    std::wstring m_worldName;
    int m_difficulty;
    int m_state614;
    int m_gameTypeId;
    int m_saveIndex;
    int m_selectedSlot;
    bool m_customGame;
    bool m_saveCorrupt;
    bool m_signInPending;
    bool m_onlineEnabled;
    bool m_touchBoxRebuild;
    bool m_state629;
    bool m_isCreative;
    bool m_state62B;
    bool m_state62C;
    int m_pendingSlot;
    long long m_seed;
};

#endif
