#ifndef GAME_UI_UISCENE_INGAMESAVEMANAGEMENTMENU_HPP
#define GAME_UI_UISCENE_INGAMESAVEMANAGEMENTMENU_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SpaceIndicatorBar.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class _SaveListDetails;
struct SAVE_DETAILS;

class UIScene_InGameSaveManagementMenu : public UIScene
{
public:
    enum EPendingAction
    {
        ePendingAction_None,
        ePendingAction_Unused,
        ePendingAction_RebuildSaveList
    };

    UIScene_InGameSaveManagementMenu(int playerId, void* data, UILayer* layer);
    virtual ~UIScene_InGameSaveManagementMenu();

    void GetSaveInfo();
    void Initialise();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void handleReload();
    virtual void handleGainFocus(bool navigatedBack);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleFocusChange(int controlId, int childId);
    virtual void handleInitFocus(int controlId, int childId);
    virtual bool hasFocus(int playerId);
    static int LoadSaveDataThumbnailReturned(void* context, unsigned char* data, unsigned int size);
    virtual void tick();
    virtual std::wstring getMoviePath();
    static int DeleteSaveDataReturned(void* context, bool success);
    static int DeleteSaveDialogReturned(void* context, int pad, C4JStorage::EMessageResult result);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();
    virtual bool hidesLowerScenes();

private:
    _SaveListDetails* m_saveListDetails;
    int m_saveDetailsCount;
    UIControl_SaveList m_saveList;
    UIControl_Label m_saveListTitle;
    UIControl m_saveTimer;
    UIControl_SpaceIndicatorBar m_saveSizeBar;
    EPendingAction m_pendingAction;
    bool m_ignoreInput;
    bool m_thumbnailsComplete;
    bool m_thumbnailRequested;
    bool m_thumbnailReturned;
    int m_nextThumbnail;
    SAVE_DETAILS* m_saveDetails;
    bool m_saveInfoReady;
    bool m_closing;
    int m_saveListRefreshState;
    int m_selectedSaveIndex;
    bool m_selectionChanged;
};

#endif
