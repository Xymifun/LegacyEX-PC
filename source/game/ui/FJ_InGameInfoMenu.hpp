#ifndef GAME_UI_FJ_INGAMEINFOMENU_HPP
#define GAME_UI_FJ_INGAMEINFOMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_InGameInfoMenu : public FJ_Document
{
public:
    FJ_InGameInfoMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
