#ifndef GAME_UI_FJ_LAUNCHMOREOPTIONSMENU_HPP
#define GAME_UI_FJ_LAUNCHMOREOPTIONSMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_LaunchMoreOptionsMenu : public FJ_Document
{
public:
    FJ_LaunchMoreOptionsMenu(fuiRenderNode* renderNode);

    void handleCheckboxToggled(FJ_Event* event);
    static void _handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void UpdateDescription();
    void SetActiveTab(int tab, bool resize);
    void ChangeTab();
    void SetMenuType(int type);
    void HideWorldTab();

private:
    int m_activeTab;
    int m_previousTab;
    int m_menuType;
    int m_gameOptionsFocus;
    int m_worldOptionsFocus;
    bool m_descriptionUpdated;
    bool m_worldTabHidden;
};

#endif
