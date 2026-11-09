#ifndef GAME_UI_FJ_CLASSICCRAFTINGMENU_HPP
#define GAME_UI_FJ_CLASSICCRAFTINGMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_ClassicCraftingMenu : public FJ_AbstractContainerDocument
{
public:
    FJ_ClassicCraftingMenu(fuiRenderNode* renderNode);
    static FJ_ClassicCraftingMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
