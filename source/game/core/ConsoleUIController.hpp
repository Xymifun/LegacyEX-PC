#ifndef GAME_CORE_CONSOLEUICONTROLLER_HPP
#define GAME_CORE_CONSOLEUICONTROLLER_HPP

#include <string>
#include <stdint.h>
#include <vector>
#include <boost/unordered_map.hpp>

#include "game/core/IUIController.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class fuiBitmapFont;
class C4JThread;
class UITTFFont;
class UIControl;
class UIScene;
class UIScene_HUD;
class UIGroup;
class Tutorial;
class _TutorialPopupInfo;
struct _CustomDrawData;

class ConsoleUIController : public IUIController
{
    friend class UIComponent_MenuBackground;
    friend class UIScene_HUD;
    friend class UIScene_SkinSelectMenu;

public:
    enum EFont
    {
        EFont_None = 0,
        EFont_Bitmap = 1,
        EFont_Japanese = 2,
        EFont_ChineseSimplified = 3,
        EFont_ChineseTraditional = 4,
        EFont_Korean = 5
    };

    struct UIELEMENT
    {
        UIControl* control;
        int left;
        int top;
        int right;
        int bottom;
    };

    struct _TextureDetail
    {
        int referenceCount;
        unsigned char* data;
        unsigned int size;
        int texture;
        bool textureLoaded;

        _TextureDetail() : referenceCount(0), data(0), size(0), texture(0), textureLoaded(false) {}
    };

    struct _CachedMovieData
    {
        arrayWithLength<unsigned char> data;
        long long expiryTime;

        _CachedMovieData() : expiryTime(0) {}
        _CachedMovieData(const arrayWithLength<unsigned char>& value, long long expiry)
            : data(value),
              expiryTime(expiry)
        {
        }
    };

    struct _MessageBoxData
    {
        int titleId;
        int messageId;
        int* messageParameters;
        unsigned int parameterCount;
        unsigned int userIndex;
        int (*callback)(void*, int, C4JStorage::EMessageResult);
        void* callbackData;
        wchar_t* overrideText;
        unsigned int buttonFlags;
        bool blocking;
    };

    struct _QueuedMessageBoxData
    {
        _MessageBoxData info;
        unsigned int queuedUserIndex;
        EUILayer layer;
    };

    ConsoleUIController();

    EUIGroup GetGroup(int playerId);
    void preInit(int width, int height);
    void postInit();
    void init(int width, int height);
    static int TextureSubstitutionCreateCallbackFui(void* context, const std::wstring& name, unsigned int* width,
                                                    unsigned int* height, bool* hasAlpha);
    static void TextureSubstitutionDestroyCallbackFui(int texture);
    EFont getFontForLanguage(int language);
    UITTFFont* createFont(EFont font);
    void updateCurrentFont();
    void SetupFont();
    intptr_t loadSkin(const std::wstring& name, const std::wstring& path, int texture);
    void loadSkins();
    void renderScenes();
    bool PendingFontChange();
    bool UsingBitmapFont();
    void TouchBoxAdd(UIControl* control, EUIGroup group, EUILayer layer, EUIScene scene, UIControl* parent);
    void TouchBoxAdd(UIControl* control, UIScene* scene);
    bool TouchBoxHit(UIScene* scene, int x, int y);
    void TouchBoxesClear(UIScene* scene);
    void TouchBoxRebuild(UIScene* scene);
    void SendTouchInput(unsigned int playerId, unsigned int action, bool pressed, bool repeat, bool released);
    void HandleTouchInput(unsigned int playerId, unsigned int action, bool pressed, bool repeat, bool released);
#ifdef MINECRAFT_PC
    void PcHoverPointer(unsigned int playerId, int x, int y);
#endif
    void updateViewportTouchOffset(C4JRender::eViewportType viewport);
    void updateViewportTouchOffset(int x, int y);
    void SetMenuDisplayed(int playerId, bool displayed);
    void setFullscreenMenuDisplayed(bool displayed);
    void handleKeyPress(unsigned int key, unsigned int character);
    bool handleInput();
    void tickInput();
    static int reloadSkinThreadProc(void* context);
    void ReloadSkin();
    arrayWithLength<unsigned char> getMovieData(const std::wstring& path);
    void getRenderDimensions(C4JRender::eViewportType viewport, int& width, int& height);
    float getScreenWidth() const { return m_screenWidth; }
    float getScreenHeight() const { return m_screenHeight; }
    void setupRenderPosition(C4JRender::eViewportType viewport);
    void setupRenderPosition(int x, int y);
    void setupCustomDrawGameState();
    void endCustomDrawGameState();
    void registerSubstitutionTexture(const std::wstring& name, int texture);
    void registerSubstitutionTexture(const std::wstring& name, unsigned char* data, unsigned int size, bool deleteData);
    void unregisterSubstitutionTexture(const std::wstring& name, bool deleteData);
    void FixupMenuDisplayedAfterNavigateBack();
    void NavigateToHomeMenu(bool immediate);
    UIScene* GetTopScene(int playerId, EUILayer layer, EUIGroup group);
    unsigned int RegisterForCallbackId(UIScene* scene);
    void UnregisterCallbackId(unsigned int callbackId);
    UIScene* GetSceneFromCallbackId(unsigned int callbackId);
    void EnterCallbackIdCriticalSection();
    void LeaveCallbackIdCriticalSection();
    void AnimateKeyPress(int playerId, int action, bool repeat, bool pressed, bool released);
    void OverrideSFX(int playerId, int action, bool enabled);
    void ResetPressStartToPlayUIComponents();
    void ClearPressStart();
    void SetAchievementUnlocked(int playerId, int achievementId, int imageId, int value);
    bool GetAchievementToastShowing(int playerId);
    bool GetHoldToShowAchievementsActive(bool includeCancelled);
    bool GetHoldActionCancelled(int playerId);
    void RequestUGCMessageBox(int titleId, int messageId, int buttonId,
                              int (*callback)(void*, int, C4JStorage::EMessageResult), void* callbackData);
    unsigned int GetWinUserIndex();
    UIScene* FindScene(EUIScene scene);

    virtual void tick();
    virtual bool requestResolutionChange(int resolution);
    virtual void setResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag);
    virtual void clearResolutionChangeDisableFlag(eRESOLUTION_DISABLE_FLAG flag);
    virtual void render();
    virtual void StartReloadSkinThread();
    virtual bool IsReloadingSkin();
    virtual void CleanUpSkinReload();
    virtual bool NavigateToScene(int playerId, EUIScene scene, void* sceneData, EUILayer layer, EUIGroup group);
    virtual bool NavigateBack(int playerId, bool closeCurrent, EUIScene scene, EUILayer layer);
    virtual void CloseUIScenes(int playerId, bool immediate);
    virtual void CloseAllPlayersScenes(bool immediate);
    virtual bool IsPauseMenuDisplayed(int playerId);
    virtual bool IsContainerMenuDisplayed(int playerId);
    virtual bool IsIgnorePlayerJoinMenuDisplayed(int playerId);
    virtual bool IsIgnoreAutosaveMenuDisplayed(int playerId);
    virtual void SetIgnoreAutosaveMenuDisplayed(int playerId, bool ignore);
    virtual bool IsSceneInStack(int playerId, EUIScene scene);
    virtual bool GetMenuDisplayed(int playerId);
    virtual void CheckMenuDisplayed();
    virtual void SetTooltipText(unsigned int playerId, unsigned int tooltipIndex, int textId);
    virtual void SetEnableTooltips(unsigned int playerId, bool enable);
    virtual void ShowTooltip(unsigned int playerId, unsigned int tooltipIndex, bool show);
    virtual void SetTooltips(unsigned int playerId, int tooltip0, int tooltip1, int tooltip2, int tooltip3,
                             int tooltip4, int tooltip5, int tooltip6, int tooltip7, int tooltip8, int tooltip9,
                             int tooltip10, int tooltip11, int tooltip12, int tooltip13, int tooltip14, int tooltip15,
                             bool show, bool useGlobalGroup);
    virtual void EnableTooltip(unsigned int playerId, unsigned int tooltipIndex, bool enable);
    virtual void RefreshTooltips(unsigned int playerId);
    virtual void PlayUISFX(const SoundEvent* soundEvent);
    virtual void ShowUIDebugConsole(bool show);
    virtual void ShowUIDebugMarketingGuide(bool show);
    virtual void DisplayGamertag(unsigned int playerId, bool show);
    virtual void SetSelectedItem(unsigned int playerId, const std::wstring& itemName,
                                 const std::wstring& itemDescription);
    virtual void ResetSelectedItem();
    virtual void UpdateSelectedItemPos(unsigned int playerId);
    virtual void HandleDLCMountingComplete();
    virtual void HandleDLCInstalled(int packId);
    virtual void HandleDLCLicenseChange();
    virtual void HandleTMSDLCFileRetrieved(int fileId);
    virtual void HandleTMSBanFileRetrieved(int fileId);
    virtual void HandleInventoryUpdated(int playerId);
    virtual void HandleGameTick();
    virtual void HandleSaveDeviceRemoved(int deviceIndex);
    virtual void SetTutorialDescription(int playerId, _TutorialPopupInfo* popupInfo);
    virtual void SetTutorialVisible(int playerId, bool visible);
    virtual bool IsTutorialVisible(int playerId);
    virtual void UpdatePlayerBasePositions();
    virtual void SetEmptyQuadrantLogo(int quadrant);
    virtual void HideAllGameUIElements();
    virtual void ShowOtherPlayersBaseScene(unsigned int playerId, bool show);
    virtual void ShowTrialTimer(bool show);
    virtual void SetTrialTimerLimitSecs(unsigned int limitSecs);
    virtual void UpdateTrialTimer(unsigned int remainingSecs);
    virtual void ReduceTrialTimerValue();
    virtual void ShowAutosaveCountdownTimer(bool show);
    virtual void UpdateAutosaveCountdownTimer(unsigned int remainingSecs);
    virtual void ShowSavingMessage(unsigned int playerId, C4JStorage::ESavingMessage message);
    virtual bool PressStartPlaying(unsigned int playerId);
    virtual void ShowPressStart(unsigned int playerId);
    virtual void ShowChestRefillAnimation(bool show);
    virtual void SetWinUserIndex(unsigned int userIndex);
    virtual void ShowTimedSplash(unsigned int playerId, const std::wstring& text, unsigned int durationMs);
    virtual bool IsShowingErrorMessage(int playerId);
    virtual bool IsShowingAlertMessage(int playerId);
    virtual bool getCleanupOnReload();
    virtual void setCleanupOnReload(bool cleanupOnReload);
    virtual bool IsExpectingOrReloadingSkin();
    virtual void SetTooltips(unsigned int playerId, int* tooltips, bool show, bool useGlobalGroup);
    virtual bool IsToolTipDynamic(unsigned int playerId, unsigned int tooltipIndex, EControllerActions& action);
    virtual int GetHotBarX(unsigned int playerId);
    virtual int GetHotBarY(unsigned int playerId);
    virtual int GetHotBarWidth(unsigned int playerId);
    virtual int GetHotBarHeight(unsigned int playerId);
    virtual bool GetQuickSelectVisible(unsigned int playerId);
    virtual bool GetDRCEnabled();
    virtual void SetTutorial(int playerId, Tutorial* tutorial);
    virtual void RemoveInteractSceneReference(int playerId, UIScene* scene);
    virtual void HideChestRefillAnimation();
    virtual void ShowPlayerDisplayname(bool show);
    virtual void HidePressStart();
    virtual int RequestProfileSignInMenu(int titleId, int messageId, wchar_t** messageParameters,
                                         unsigned int parameterCount, unsigned int userIndex,
                                         int (*callback)(void*, int, int), void* callbackData, wchar_t* overrideText,
                                         unsigned int controllerIndex);
    virtual void RequestAlertMessage(int titleId, int messageId, int* messageParameters, unsigned int parameterCount,
                                     unsigned int userIndex, int (*callback)(void*, int, C4JStorage::EMessageResult),
                                     void* callbackData, wchar_t* overrideText, bool blocking);
    virtual int RequestErrorMessage(int titleId, int messageId, int* messageParameters, unsigned int parameterCount,
                                    unsigned int userIndex, int (*callback)(void*, int, C4JStorage::EMessageResult),
                                    void* callbackData, wchar_t* overrideText, bool blocking);
    virtual int RequestMessageBox(int titleId, int messageId, int* messageParameters, unsigned int parameterCount,
                                  unsigned int userIndex, int (*callback)(void*, int, C4JStorage::EMessageResult),
                                  void* callbackData, wchar_t* overrideText, unsigned int buttonFlags, bool alert,
                                  bool blocking);

    void shutdown();
    void setBlockOrigin(int x, int y);
    static void handleUnlockFullVersionCallback();
    UIScene_HUD* GetHUD(unsigned int playerId);
    void showComponent(int playerId, EUIScene scene, EUILayer layer, EUIGroup group, bool show);
    void removeComponent(EUIScene scene, EUILayer layer, EUIGroup group);
    void SetSysUIShowing(bool showing);
    static void SetSystemUIShowing(void* context, bool showing);
    void setupCustomDrawMatrices(UIScene* scene, _CustomDrawData* data);
    void setupCustomDrawGameStateAndMatrices(UIScene* scene, _CustomDrawData* data);
    void endCustomDrawMatrices();
    void endCustomDrawGameStateAndMatrices();
    void logDebugString(const std::string& text);

private:
    UIScene* m_debugConsole;
    int m_layoutWord;
    OSFastMutex m_renderMutex;
    OSFastMutex m_messageBoxMutex;
    OSFastMutex m_touchMutex;
    int m_actionTimer[9][0x18];
    float m_screenWidth;
    float m_screenHeight;
    bool m_initialized;
    int m_renderOffsetX;
    int m_renderOffsetY;
    OSFastMutex m_fontMutex;
    int m_currentFont;
    EFont m_pendingFont;
    bool m_cleanupOnReload;
    bool m_autoCleanupOnReload;
    bool m_fontChangePending;
    UITTFFont* m_font;
    UITTFFont* m_oldFont;
    fuiBitmapFont* m_bitmapFont;
    fuiBitmapFont* m_bitmapFontAlt;
    unsigned int m_resolutionDisableFlags;
    std::vector<UIELEMENT*> m_touchBoxes[EUIGroup_Count][EUILayer_Count][EUIScene_None];
    int m_touchHoldPad;
    bool m_touchHeld;
    int m_touchX;
    int m_touchY;
    bool m_rebuildingTouchBoxes;
    intptr_t m_skinScenes[11];
    bool m_menuDisplayed[4];
    bool m_menuDisplayPending[4];
    int m_menuDisplayDelay[4];
    bool m_playerFlags[6];
    unsigned int m_pressStartPlaying;

public:
    unsigned int m_pressStartBlocked;

private:
    int m_lastViewport;
    bool m_viewportPositionDirty;
    boost::unordered_map<std::wstring, _TextureDetail> m_substitutionTextures;
    boost::unordered_map<std::wstring, _CachedMovieData> m_movieCache;
    std::vector<_QueuedMessageBoxData*> m_queuedMessageBoxes;
    unsigned int m_winUserIndex;
    bool m_inputSuspended;
    C4JThread* m_skinThread;
    OSFastMutex m_skinThreadMutex;
    bool m_homeMenuPending;
    unsigned int m_lastInputTime;
    unsigned long long m_lastUISFXTime;
    int m_customDrawLeft;
    int m_customDrawTop;
    int m_customDrawRight;
    int m_customDrawBottom;
    boost::unordered_map<unsigned int, UIScene*> m_callbackScenes;
    OSFastMutex m_callbackMutex;
    int m_touchOffsetX;
    int m_touchOffsetY;
    UIELEMENT* m_touchPressElement;
    UIELEMENT* m_touchCurrentElement;
    UIGroup* m_groups[EUIGroup_Count];
};

extern OSFastMutex g_uiCriticalSection;
extern ConsoleUIController gConsoleUIController;
extern uint8_t g_introRunning;

void PopupToast(void* context, int playerId, int achievementId, int imageId, int value);

#endif
