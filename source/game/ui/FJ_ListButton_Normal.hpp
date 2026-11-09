#ifndef GAME_UI_FJ_LISTBUTTON_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTON_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButton.hpp"

class fuiRenderNode;

class FJ_ListButton_Normal : public FJ_ListButton
{
public:
    explicit FJ_ListButton_Normal(fuiRenderNode* renderNode);
    static FJ_ListButton* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
