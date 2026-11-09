#ifndef GAME_UI_FJ_BREWINGSTANDMENU_HPP
#define GAME_UI_FJ_BREWINGSTANDMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_BrewingStandMenu : public FJ_AbstractContainerDocument
{
public:
    FJ_BrewingStandMenu(fuiRenderNode* renderNode);
    static FJ_BrewingStandMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
