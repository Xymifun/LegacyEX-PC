#ifndef GAME_UI_FJ_LOADMENU_HPP
#define GAME_UI_FJ_LOADMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_LoadMenu : public FJ_Document
{
public:
    FJ_LoadMenu(fuiRenderNode* renderNode);

    void handleCheckboxToggled(FJ_Event* event);
    static void _handleCheckboxToggled(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    void InitCustomLoadMenu();
    void RescaleCustomLoadMenu(bool comparison);
    static FJ_LoadMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();

private:
    int m_unused;
    bool m_customLoadMenu;
    int m_listHeight;
    int m_iconY;
};

#endif
