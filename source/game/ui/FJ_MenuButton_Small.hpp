#ifndef GAME_UI_FJ_MENUBUTTON_SMALL_HPP
#define GAME_UI_FJ_MENUBUTTON_SMALL_HPP

#include <stdint.h>

#include "game/ui/FJ_MenuButton.hpp"

class fuiRenderNode;

class FJ_MenuButton_Small : public FJ_MenuButton
{
public:
    explicit FJ_MenuButton_Small(fuiRenderNode* renderNode);

    static FJ_MenuButton* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
