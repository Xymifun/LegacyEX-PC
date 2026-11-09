#ifndef GAME_UI_FJ_LOADCREATEJOINMENU_HPP
#define GAME_UI_FJ_LOADCREATEJOINMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_LoadCreateJoinMenu : public FJ_Document
{
public:
    FJ_LoadCreateJoinMenu(fuiRenderNode* renderNode);
    void SetActiveTab(int tab);
    static FJ_LoadCreateJoinMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void handlePress(FJ_Event* event);
    void SetMatchesAvailable(bool available);
    void ShowSaveSizeBar(bool show);
    void HideLoadTab();

private:
    int m_activeTab;
    int m_currentTab;
    int m_loadTabFocus;
    int m_newTabFocus;
    int m_joinTabFocus;
    int m_loadListHeight;
    int m_loadListRecessHeight;
    int m_loadTimerY;
    bool m_touchEnabled;
    int m_loadTabTitleY;
    int m_joinTabTitleY;
    int m_newTabX;
    int m_joinTabX;
};

#endif
