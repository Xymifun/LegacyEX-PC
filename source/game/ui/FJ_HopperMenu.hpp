#ifndef GAME_UI_FJ_HOPPERMENU_HPP
#define GAME_UI_FJ_HOPPERMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_HopperMenu : public FJ_AbstractContainerDocument
{
public:
    FJ_HopperMenu(fuiRenderNode* renderNode);

    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
