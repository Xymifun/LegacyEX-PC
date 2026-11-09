#ifndef GAME_CORE_IUICONTROLLER_HPP
#define GAME_CORE_IUICONTROLLER_HPP

#include <string>

#include "game/core/GameSettings.hpp"

#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class SoundEvent;
class Tutorial;
class _TutorialPopupInfo;

class IUIController
{
public:
    enum eRESOLUTION_DISABLE_FLAG
    {
        RESOLUTION_DISABLE_FLAG_MOVIE_PLAYING,
        RESOLUTION_DISABLE_FLAG_SKIN_RELOAD,
        RESOLUTION_DISABLE_FLAG_SAVING
    };

    virtual ~IUIController() {}

    virtual void tick() = 0;
    virtual bool requestResolutionChange(int resolution) = 0;
    virtual void setResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag) = 0;
    virtual void clearResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag) = 0;
    virtual void render() = 0;
    virtual void StartReloadSkinThread() = 0;
    virtual bool IsReloadingSkin() = 0;
    virtual void CleanUpSkinReload() = 0;
    virtual bool NavigateToScene(int playerId, EUIScene scene, void* pSceneData, EUILayer layer, EUIGroup group) = 0;
    virtual bool NavigateBack(int playerId, bool bCloseCurrent, EUIScene scene, EUILayer layer) = 0;
    virtual void CloseUIScenes(int playerId, bool bImmediate) = 0;
    virtual void CloseAllPlayersScenes(bool bImmediate) = 0;
    virtual bool IsPauseMenuDisplayed(int playerId) = 0;
    virtual bool IsContainerMenuDisplayed(int playerId) = 0;
    virtual bool IsIgnorePlayerJoinMenuDisplayed(int playerId) = 0;
    virtual bool IsIgnoreAutosaveMenuDisplayed(int playerId) = 0;
    virtual void SetIgnoreAutosaveMenuDisplayed(int playerId, bool bIgnore) = 0;
    virtual bool IsSceneInStack(int playerId, EUIScene scene) = 0;
    virtual bool GetMenuDisplayed(int playerId) = 0;
    virtual void CheckMenuDisplayed() = 0;
    virtual void SetTooltipText(unsigned int playerId, unsigned int tooltipIndex, int textId) = 0;
    virtual void SetEnableTooltips(unsigned int playerId, bool bEnable) = 0;
    virtual void ShowTooltip(unsigned int playerId, unsigned int tooltipIndex, bool bShow) = 0;
    virtual void SetTooltips(unsigned int playerId, int tooltip0, int tooltip1, int tooltip2, int tooltip3,
                             int tooltip4, int tooltip5, int tooltip6, int tooltip7, int tooltip8, int tooltip9,
                             int tooltip10, int tooltip11, int tooltip12, int tooltip13, int tooltip14, int tooltip15,
                             bool bShow, bool bUseGlobalGroup) = 0;
    virtual void EnableTooltip(unsigned int playerId, unsigned int tooltipIndex, bool bEnable) = 0;
    virtual void RefreshTooltips(unsigned int playerId) = 0;
    virtual void PlayUISFX(const SoundEvent* pSoundEvent) = 0;
    virtual void ShowUIDebugConsole(bool bShow);
    virtual void ShowUIDebugMarketingGuide(bool bShow);
    virtual void DisplayGamertag(unsigned int playerId, bool bShow) = 0;
    virtual void SetSelectedItem(unsigned int playerId, const std::wstring& itemName,
                                 const std::wstring& itemDescription) = 0;
    virtual void ResetSelectedItem() = 0;
    virtual void UpdateSelectedItemPos(unsigned int playerId) = 0;
    virtual void HandleDLCMountingComplete() = 0;
    virtual void HandleDLCInstalled(int packId) = 0;
    virtual void HandleDLCLicenseChange() = 0;
    virtual void HandleTMSDLCFileRetrieved(int fileId) = 0;
    virtual void HandleTMSBanFileRetrieved(int fileId) = 0;
    virtual void HandleInventoryUpdated(int playerId) = 0;
    virtual void HandleGameTick() = 0;
    virtual void HandleSaveDeviceRemoved(int deviceIndex) = 0;
    virtual void SetTutorialDescription(int playerId, _TutorialPopupInfo* pPopupInfo) = 0;
    virtual void SetTutorialVisible(int playerId, bool bVisible) = 0;
    virtual bool IsTutorialVisible(int playerId) = 0;
    virtual void UpdatePlayerBasePositions() = 0;
    virtual void SetEmptyQuadrantLogo(int quadrant) = 0;
    virtual void HideAllGameUIElements() = 0;
    virtual void ShowOtherPlayersBaseScene(unsigned int playerId, bool bShow) = 0;
    virtual void ShowTrialTimer(bool bShow) = 0;
    virtual void SetTrialTimerLimitSecs(unsigned int limitSecs) = 0;
    virtual void UpdateTrialTimer(unsigned int remainingSecs) = 0;
    virtual void ReduceTrialTimerValue() = 0;
    virtual void ShowAutosaveCountdownTimer(bool bShow) = 0;
    virtual void UpdateAutosaveCountdownTimer(unsigned int remainingSecs) = 0;
    virtual void ShowSavingMessage(unsigned int playerId, C4JStorage::ESavingMessage savingMessage) = 0;
    virtual bool PressStartPlaying(unsigned int playerId) = 0;
    virtual void ShowPressStart(unsigned int playerId) = 0;
    virtual void ShowChestRefillAnimation(bool bShow) = 0;
    virtual void SetWinUserIndex(unsigned int userIndex) = 0;
    virtual void ShowTimedSplash(unsigned int playerId, const std::wstring& text, unsigned int durationMs) = 0;
    virtual bool IsShowingErrorMessage(int playerId) = 0;
    virtual bool IsShowingAlertMessage(int playerId) = 0;
    virtual bool getCleanupOnReload() = 0;
    virtual void setCleanupOnReload(bool bCleanupOnReload) = 0;
    virtual bool IsExpectingOrReloadingSkin() = 0;
};

#endif
