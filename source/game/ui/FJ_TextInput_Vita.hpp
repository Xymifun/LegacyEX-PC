#ifndef GAME_UI_FJ_TEXTINPUT_VITA_HPP
#define GAME_UI_FJ_TEXTINPUT_VITA_HPP

#include "game/ui/FJ_TextInput.hpp"

class FJ_TextInput_Vita : public FJ_TextInput
{
public:
    explicit FJ_TextInput_Vita(fuiRenderNode* renderNode);

    static FJ_TextInput* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
