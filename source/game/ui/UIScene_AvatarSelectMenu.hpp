#ifndef GAME_UI_UISCENE_AVATARSELECTMENU_HPP
#define GAME_UI_UISCENE_AVATARSELECTMENU_HPP

#include <vector>
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SaveList.hpp"

struct MiiDataEntry;
class UILayer;

class UIScene_AvatarSelectMenu : public UIScene
{
public:
    class TabControls
    {
    public:
        TabControls();
        virtual void ShowTimer(bool show);
        UIControl m_tab;
        UIControl_SaveList m_list;
        UIControl_Label m_title;
        UIControl m_timer;
        UIScene_AvatarSelectMenu* m_owner;
        std::vector<MiiDataEntry*> m_miis;
    };

    UIScene_AvatarSelectMenu(int player, void* data, UILayer* layer);
    virtual ~UIScene_AvatarSelectMenu();
    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleDestroy();
    virtual void handleLoseFocus();
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleTimerComplete(int timerId);
    bool handleKeyDown(int player, int action, bool repeat);
    virtual void handleInput(int player, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handleInitFocus(int focus, int childFocus);
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual UIControl* GetMainPanel();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();
    virtual unsigned int GetPad();
    virtual void SelectTab(int tab);
    void AddMiiItem(TabControls& controls, MiiDataEntry* mii);
    void AddMiiItems();
    void Initialise();

private:
    UIControl_Touch m_userTabTouch;
    UIControl_Touch m_favouriteTabTouch;
    UIControl_Touch m_guestTabTouch;
    int m_currentTab;
    bool m_initialised;
    bool m_unknown1;
    bool m_navigateBack;
    TabControls m_userControls;
    TabControls m_favouriteControls;
    TabControls m_guestControls;
    bool m_miisAdded;
    int m_userFocus;
    int m_favouriteFocus;
    int m_guestFocus;
    int m_pad;
    int m_choice;
    int m_activeTab;
};

#endif
