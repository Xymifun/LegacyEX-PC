#ifndef GAME_CORE_IUISCENE_LOADCREATEJOIN_HPP
#define GAME_CORE_IUISCENE_LOADCREATEJOIN_HPP
#include <stdint.h>
#include <string>
#include <vector>
#include <coreinit/fastmutex.h>
#include "game/core/FileHeader.hpp"
#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/core/IUIScene_DLCUpsell.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "platform/wiiu/C4JStorage.hpp"
class File;
class _SaveListDetails;
class LevelGenerationOptions;
struct FriendSessionInfo;
struct SAVE_DETAILS;
struct JoinSceneData
{
    FriendSessionInfo* session;
    int userIndex;
    bool miniGames;
};
class IUIScene_LoadCreateJoin : public IUIScene_DLCUpsell, public IUIPartial_OnlineRestricted
{
public:
    enum EPendingAction
    {
        ePendingAction_None,
        ePendingAction_GetSaveInfo,
        ePendingAction_RebuildSaveList,
        ePendingAction_ReloadSaveInfo,
        ePendingAction_ReloadSaveInfoUnused
    };

    IUIScene_LoadCreateJoin();
    ~IUIScene_LoadCreateJoin();
    void UpdateGamesList();
    static void UpdateGamesListCallback(void*);
    void _init(bool);
    void AddMatchmakingButton(std::wstring, int, std::wstring);
    void AddGameModeButton(std::wstring, int, std::wstring);
    void AddDefaultButtons();
    static int HandleTelemetrySavesReturned(void*, SAVE_DETAILS*, bool);
    void GetSaveInfo();
    void _tick();
    void _updateTooltips();
    void handleTimer(int);
    static int DeleteSaveDialogReturned(void*, int, C4JStorage::EMessageResult);
    static int SaveOptionsDialogReturned(void*, int, C4JStorage::EMessageResult);
    bool handleKeyDown(int, int, bool);
    void LoadLevelGen(LevelGenerationOptions*);
    void LoadSaveFromDisk(File*, ESavePlatform);
    void LoadGameMode(EMiniGameId);
    void HandleLoadListPress(int);
    void HandleNewListPress(int, int);
    static int TexturePackDialogReturned(void*, int, C4JStorage::EMessageResult);
    void CheckAndJoinGame(int);
    void HandleGamesListPressed(int);
    virtual void SetIgnoreInput(bool);
    virtual void Initialise() = 0;
    virtual void updateTooltips() = 0;
    virtual int GetPad() = 0;
    virtual void SelectTab(int tab) = 0;
    virtual void SetMatchesAvailable(bool available) = 0;
    virtual void SetHideLoadTab() = 0;
    virtual bool GetHideLoadTab() = 0;
    virtual void AddLoadListButton(const std::wstring& label, int id) = 0;
    virtual bool DoesLoadListHaveFocus() = 0;
    virtual void SetLoadListIcon(int index, unsigned char* data, unsigned int size, const std::wstring& name) = 0;
    virtual void ScrollLoadListToSelection(unsigned int selection) = 0;
    virtual void ClearLoadList() = 0;
    virtual unsigned int GetLoadListSize() = 0;
    virtual unsigned int GetLoadListSelection() = 0;
    virtual void SetLoadListEnabled(bool enabled) = 0;
    virtual void AddCreateListButton(const std::wstring& label, int id) = 0;
    virtual bool DoesCreateListHaveFocus() = 0;
    virtual bool DoesMashUpWorldHaveFocus() = 0;
    virtual void SetCreateListIcon(int index, unsigned char* data, unsigned int size, const std::wstring& name) = 0;
    virtual void ScrollCreateListToSelection(unsigned int selection) = 0;
    virtual void ClearCreateList() = 0;
    virtual void SetCreateListEnabled(bool enabled) = 0;
    virtual void AddJoinListButton(const std::wstring& label, int id, unsigned char* data, unsigned int size,
                                   const std::wstring& textureName, unsigned char* data1, unsigned int size1,
                                   const std::wstring& textureName1) = 0;
    virtual bool DoesGamesListHaveFocus() = 0;
    virtual void InitFocusJoinList(int focus) = 0;
    virtual void ClearJoinList() = 0;
    virtual unsigned int GetJoinListSize() = 0;
    virtual unsigned int GetJoinListSelection() = 0;
    virtual bool IsJoinListEnabled() = 0;
    virtual void SetJoinListEnabled(bool enabled) = 0;
    virtual void SetGamesListSelection(unsigned int selection) = 0;
    virtual void ShowSaveListTimer(bool show) = 0;
    virtual void ShowJoinListTimer(bool show) = 0;
    virtual void ShowNoGamesLabel(bool show) = 0;
    virtual void DeleteSaveFile(int index) = 0;
    virtual int handleSaveOptionsDialogReturned(int userIndex, C4JStorage::EMessageResult result) = 0;
    virtual int handleTexturePackDialogReturned(int userIndex, C4JStorage::EMessageResult result) = 0;
    virtual uint32_t GetCustomGameModeID();

protected:
    std::vector<FriendSessionInfo*>* m_friendSessions;
    _SaveListDetails* m_saveListDetails;
    SAVE_DETAILS* m_saveDetails;
    JoinSceneData* m_joinSceneData;
    bool m_ignoreInput;
    bool m_onlinePlayAllowed;
    bool m_thumbnailRequested;
    bool m_saveInfoReady;
    bool m_thumbnailsComplete;
    bool m_closing;
    bool m_partyGamesAvailable;
    bool m_partyGamesOnly;
    bool m_miniGames;
    int m_matchmakingButtonCount;
    EPendingAction m_pendingAction;
    int m_nextThumbnail;
    int m_saveListRefreshState;
    int m_buttonCount;
    int m_texturePackWorldCount;
    int m_selectedSaveIndex;
    int m_selectedGameIndex;
    int m_selectedCreateIndex;
    int m_pressedList;
    std::vector<LevelGenerationOptions*> m_levelGenerationOptions;
    std::vector<File*>* m_debugSaveFiles;
    std::vector<int> m_buttonIds;
    bool m_sessionListMode;
    int m_currentTab;
    EMiniGameId m_selectedMiniGame;
    bool m_returnToHomeMenu;
    OSFastMutex m_lock;
};
#endif
