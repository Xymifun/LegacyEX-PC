#ifndef GAME_UI_FJ_CHECKBOX_NORMAL_HPP
#define GAME_UI_FJ_CHECKBOX_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_CheckBox.hpp"

class fuiRenderNode;

class FJ_CheckBox_Normal : public FJ_CheckBox
{
public:
    explicit FJ_CheckBox_Normal(fuiRenderNode* renderNode);
    static FJ_CheckBox* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
