#ifndef GAME_UI_FJ_TEXTINPUT_SMALL_HPP
#define GAME_UI_FJ_TEXTINPUT_SMALL_HPP

#include "game/ui/FJ_TextInput.hpp"

class fuiRenderNode;

class FJ_TextInput_Small : public FJ_TextInput
{
public:
    explicit FJ_TextInput_Small(fuiRenderNode* renderNode);
    static FJ_TextInput* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
