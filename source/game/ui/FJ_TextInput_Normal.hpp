#ifndef GAME_UI_FJ_TEXTINPUT_NORMAL_HPP
#define GAME_UI_FJ_TEXTINPUT_NORMAL_HPP

#include "game/ui/FJ_TextInput.hpp"

class fuiRenderNode;

class FJ_TextInput_Normal : public FJ_TextInput
{
public:
    explicit FJ_TextInput_Normal(fuiRenderNode* renderNode);
    static FJ_TextInput* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
