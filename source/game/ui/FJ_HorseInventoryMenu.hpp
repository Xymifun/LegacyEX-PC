#ifndef GAME_UI_FJ_HORSEINVENTORYMENU_HPP
#define GAME_UI_FJ_HORSEINVENTORYMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_HorseInventoryMenu : public FJ_AbstractContainerDocument
{
public:
    FJ_HorseInventoryMenu(fuiRenderNode* renderNode);

    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetInventoryType(int type);
    void SetHasInventory(bool hasInventory);
    void SetCanHaveSaddle(bool canHaveSaddle);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
