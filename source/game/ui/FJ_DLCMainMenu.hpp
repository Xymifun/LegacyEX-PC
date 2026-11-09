#ifndef GAME_UI_FJ_DLCMAINMENU_HPP
#define GAME_UI_FJ_DLCMAINMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;
class FJ_Event;

class FJ_DLCMainMenu : public FJ_Document
{
public:
    FJ_DLCMainMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void handlePress(FJ_Event* event);
};

#endif
