#ifndef PLATFORM_PC_PCFRIENDSMENU_HPP
#define PLATFORM_PC_PCFRIENDSMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SaveList.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/ui/UIScene.hpp"

void PcOpenProfile(int pad, const std::wstring& name, const unsigned char uuid[16]);

class UIScene_PcFriendsMenu : public UIScene
{
public:
    UIScene_PcFriendsMenu(int player, void* data, UILayer* layer);

    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleGainFocus(bool navigatedBack);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleFocusChange(int controlId, int childId);
    virtual void handleTouchInput(unsigned int playerId, int x, int y, int box, bool pressed, bool repeat,
                                  bool released);
    virtual UIControl* GetMainPanel();

private:
    void SetTab(int t);
    void Layout();
    void ShowText();
    void FillFriends();
    void FillRequests();
    std::wstring HeadFor(unsigned int skin);
    void Leave();

    UIControl m_tabs[3];
    UIControl_Label m_titles[3];
    UIControl_Touch m_touch[3];
    UIControl m_timers[3];
    UIControl_Label m_text;
    UIControl_Button m_buttons[4];
    UIControl_Label m_empty[2];
    UIControl_SaveList m_friendList;
    unsigned int m_friendsRevision;
    UIControl_SaveList m_requestList;
    unsigned int m_requestsRevision;
    std::vector<std::wstring> m_labels;
    std::vector<std::wstring> m_plain;
    int m_focused;
    float m_titleX[3];
    int m_tab;
    bool m_inGame;
    bool m_left;
};

#endif
