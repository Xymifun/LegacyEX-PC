#ifndef GAME_UI_UISCENE_LOADCREATEJOINMENU_HPP
#define GAME_UI_UISCENE_LOADCREATEJOINMENU_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LeftIconButtonList.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UIControl_SpaceIndicatorBar.hpp"
#include "game/ui/UIControl_Touch.hpp"

class UILayer;

class UIScene_LoadCreateJoinMenu : public IUIScene_LoadCreateJoin, public UIScene
{
public:
    enum EUploadState
    {
        eUploadState_Finished,
        eUploadState_LoadingSave,
        eUploadState_SaveLoaded,
        eUploadState_StartingLanSession,
        eUploadState_Cancelled,
        eUploadState_WaitingForReceiver,
        eUploadState_Transferring,
        eUploadState_Complete,
        eUploadState_Unused,
        eUploadState_Failed,
        eUploadState_CompleteShown,
        eUploadState_FailedShown
    };

    UIScene_LoadCreateJoinMenu(int playerId, void* data, UILayer* layer);
    virtual unsigned int mapElementsAndNames();
    virtual ~UIScene_LoadCreateJoinMenu();
    virtual void updateComponents();
    virtual void handleDestroy();
    virtual void handleLoseFocus();
    virtual void handleGainFocus(bool navigatedBack);
    virtual void handleTimerComplete(int timerId);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleFocusChange(int controlId, int childId);
    virtual void handleInitFocus(int controlId, int childId);
    virtual int getSceneType();
    virtual unsigned int getSubSceneType() const;
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual UIControl* GetMainPanel();
    virtual void tick();
    virtual void handleReload();
    virtual void HandleDLCInstalled();
    virtual std::wstring getMoviePath();
    virtual void handlePress(int controlId, int childId);
    virtual void HandleDLCMountingComplete();
    virtual void HandleSaveDeviceRemoved(int deviceIndex);

    wchar_t* getThumbnailName(int index);
    static int LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size);
    void requestNextThumbnail();
    void cleanUpThumbNails(bool all);
    virtual void updateTooltips();
    virtual void SelectTab(int tab);
    virtual void SetMatchesAvailable(bool available);
    virtual void SetHideLoadTab();
    virtual bool GetHideLoadTab();
    virtual void AddLoadListButton(const std::wstring& label, int id);
    virtual void SetLoadListIcon(int index, unsigned char* data, unsigned int size, const std::wstring& name);
    virtual void ScrollLoadListToSelection(unsigned int selection);
    virtual void ClearLoadList();
    virtual unsigned int GetLoadListSize();
    virtual unsigned int GetLoadListSelection();
    virtual void SetLoadListEnabled(bool enabled);
    virtual void AddCreateListButton(const std::wstring& label, int id);
    virtual bool DoesCreateListHaveFocus();
    virtual void SetCreateListIcon(int index, unsigned char* data, unsigned int size, const std::wstring& name);
    virtual void ScrollCreateListToSelection(unsigned int selection);
    virtual void ClearCreateList();
    virtual void SetCreateListEnabled(bool enabled);
    virtual void AddJoinListButton(const std::wstring& label, int id, unsigned char* gameIcon,
                                   unsigned int gameIconSize, const std::wstring& gameIconName, unsigned char* packIcon,
                                   unsigned int packIconSize, const std::wstring& packIconName);
    virtual void InitFocusJoinList(int focus);
    virtual void ClearJoinList();
    virtual unsigned int GetJoinListSize();
    virtual unsigned int GetJoinListSelection();
    virtual bool IsJoinListEnabled();
    virtual void SetJoinListEnabled(bool enabled);
    virtual void SetGamesListSelection(unsigned int selection);
    virtual void ShowSaveListTimer(bool show);
    virtual void ShowJoinListTimer(bool show);
    virtual void ShowNoGamesLabel(bool show);
    static int DeleteSaveDataReturned(void* context, bool success);
    virtual void DeleteSaveFile(int index);
    virtual void Initialise();
    virtual bool DoesLoadListHaveFocus();
    virtual bool DoesMashUpWorldHaveFocus();
    virtual bool DoesGamesListHaveFocus();
    static int RenameSaveDataReturned(void* context, bool success);
    static int KeyboardCompleteWorldNameCallback(void* context, bool accepted);
    static int CopySaveErrorDialogFinishedCallback(void* context, int pad, C4JStorage::EMessageResult result);
    static int CopySaveDataReturned(void* context, bool success, C4JStorage::ESaveGameState state);
    static bool CopySaveDataProgress(void* context, int percent);
    static int CopySaveThreadProc(void* context);
    static int CopySaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    static int LoadSaveDataForUploadReturned(void* context, bool failed, bool complete);
    static int CrossSaveUploadFinishedCallback(void* context, int pad, C4JStorage::EMessageResult result);
    static int UploadNintendoLanSaveThreadProc(void* context);
    static void CancelSaveUploadCallback(void* context);
    void LaunchSaveUpload();
    static int SaveTransferDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    virtual int handleSaveOptionsDialogReturned(int pad, C4JStorage::EMessageResult result);
    virtual int handleTexturePackDialogReturned(int pad, C4JStorage::EMessageResult result);
    virtual int GetPad();
    static void LoadSaveCallback(void* context, bool success);

private:
    UIControl_Touch m_loadTouch;
    UIControl_Touch m_createTouch;
    UIControl_Touch m_joinTouch;
    int m_saveDetailsCount;
    bool m_hideLoadTab;
    bool m_miniGamesMenu;
    bool m_saveDeviceRemoved;
    UIControl m_loadTab;
    UIControl m_createTab;
    UIControl m_joinTab;
    UIControl_SaveList m_loadList;
    UIControl_SaveList m_createList;
    UIControl_LeftIconButtonList m_gamesList;
    UIControl_Label m_loadTitle;
    UIControl_Label m_createTitle;
    UIControl_Label m_joinTitle;
    UIControl m_loadTimer;
    UIControl m_createTimer;
    UIControl m_joinTimer;
    UIControl_SpaceIndicatorBar m_saveSizeBar;
    int m_thumbnailWindowStart;
    bool m_thumbnailReturned;
    int m_unusedCounter;
    bool m_copyInProgress;
    bool m_copyCancelled;
    bool m_unusedFlagA;
    bool m_unusedFlagB;
    bool m_loadSelectionChanged;
    int m_activeTab;
    EUploadState m_uploadState;
    bool m_uploadCancelled;
};

#endif
