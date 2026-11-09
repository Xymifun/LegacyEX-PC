#ifndef GAME_UI_FJ_DLCOFFERSMENU_HPP
#define GAME_UI_FJ_DLCOFFERSMENU_HPP

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class fuiRenderNode;

class FJ_DLCOffersMenu : public FJ_Document
{
public:
    FJ_DLCOffersMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void handlePress(FJ_Event* event);
};

#endif
