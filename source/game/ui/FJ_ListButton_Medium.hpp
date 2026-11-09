#ifndef GAME_UI_FJ_LISTBUTTON_MEDIUM_HPP
#define GAME_UI_FJ_LISTBUTTON_MEDIUM_HPP

#include <stdint.h>
#include "game/ui/FJ_ListButton.hpp"

class fuiRenderNode;

class FJ_ListButton_Medium : public FJ_ListButton
{
public:
    explicit FJ_ListButton_Medium(fuiRenderNode* renderNode);
    static FJ_ListButton* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
