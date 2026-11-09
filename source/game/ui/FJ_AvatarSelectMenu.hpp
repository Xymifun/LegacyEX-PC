#ifndef GAME_UI_FJ_AVATARSELECTMENU_HPP
#define GAME_UI_FJ_AVATARSELECTMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_AvatarSelectMenu : public FJ_Document
{
private:
    int m_activeTab;
    int m_currentTab;
    int m_userTabFocus;
    int m_favouriteTabFocus;
    int m_guestTabFocus;

public:
    FJ_AvatarSelectMenu(fuiRenderNode* renderNode);
    static FJ_AvatarSelectMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void handlePress(FJ_Event* event);
    void SetActiveTab(int tab);
};

#endif
