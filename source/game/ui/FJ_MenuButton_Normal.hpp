#ifndef GAME_UI_FJ_MENUBUTTON_NORMAL_HPP
#define GAME_UI_FJ_MENUBUTTON_NORMAL_HPP

#include "game/ui/FJ_MenuButton.hpp"

class fuiRenderNode;

class FJ_MenuButton_Normal : public FJ_MenuButton
{
public:
    explicit FJ_MenuButton_Normal(fuiRenderNode* renderNode);

    static FJ_MenuButton* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
