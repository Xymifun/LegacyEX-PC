#ifndef GAME_UI_FJ_LISTBUTTONCHECKBOX_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONCHECKBOX_SMALL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonCheckbox.hpp"

class fuiRenderNode;

class FJ_ListButtonCheckbox_Small : public FJ_ListButtonCheckbox
{
public:
    explicit FJ_ListButtonCheckbox_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonCheckbox* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
