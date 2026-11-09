#ifndef GAME_UI_FJ_LABEL_WHITE_SMALL_CENTERED_HPP
#define GAME_UI_FJ_LABEL_WHITE_SMALL_CENTERED_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_White_Small_Centered : public FJ_Label
{
public:
    FJ_Label_White_Small_Centered(fuiRenderNode* pRenderNode) : FJ_Label(pRenderNode) {}
    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
