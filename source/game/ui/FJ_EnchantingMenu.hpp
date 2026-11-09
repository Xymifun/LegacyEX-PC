#ifndef GAME_UI_FJ_ENCHANTINGMENU_HPP
#define GAME_UI_FJ_ENCHANTINGMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_EnchantingMenu : public FJ_AbstractContainerDocument
{
public:
    FJ_EnchantingMenu(fuiRenderNode* renderNode);
    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
