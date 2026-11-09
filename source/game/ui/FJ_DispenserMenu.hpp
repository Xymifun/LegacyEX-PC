#ifndef GAME_UI_FJ_DISPENSERMENU_HPP
#define GAME_UI_FJ_DISPENSERMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_DispenserMenu : public FJ_AbstractContainerDocument
{
public:
    explicit FJ_DispenserMenu(fuiRenderNode* renderNode);
    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
