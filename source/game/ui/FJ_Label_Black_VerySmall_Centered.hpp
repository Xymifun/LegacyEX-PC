#ifndef GAME_UI_FJ_LABEL_BLACK_VERYSMALL_CENTERED_HPP
#define GAME_UI_FJ_LABEL_BLACK_VERYSMALL_CENTERED_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Black_VerySmall_Centered : public FJ_Label
{
public:
    FJ_Label_Black_VerySmall_Centered(fuiRenderNode* pRenderNode) : FJ_Label(pRenderNode) {}
    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
