#ifndef GAME_UI_FJ_INGAMETELEPORTMENU_HPP
#define GAME_UI_FJ_INGAMETELEPORTMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_InGameTeleportMenu : public FJ_Document
{
public:
    FJ_InGameTeleportMenu(fuiRenderNode* renderNode);
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
