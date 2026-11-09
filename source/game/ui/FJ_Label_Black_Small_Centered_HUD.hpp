#ifndef GAME_UI_FJ_LABEL_BLACK_SMALL_CENTERED_HUD_HPP
#define GAME_UI_FJ_LABEL_BLACK_SMALL_CENTERED_HUD_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Black_Small_Centered_HUD : public FJ_Label
{
public:
    FJ_Label_Black_Small_Centered_HUD(fuiRenderNode* pRenderNode) : FJ_Label(pRenderNode) {}
    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
