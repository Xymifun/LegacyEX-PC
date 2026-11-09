#ifndef GAME_UI_FJ_CHECKBOX_SMALL_HPP
#define GAME_UI_FJ_CHECKBOX_SMALL_HPP

#include "game/ui/FJ_CheckBox.hpp"

class fuiRenderNode;

class FJ_CheckBox_Small : public FJ_CheckBox
{
public:
    FJ_CheckBox_Small(fuiRenderNode* pRenderNode);
    static FJ_CheckBox* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
