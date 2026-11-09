#ifndef GAME_UI_FJ_LISTBUTTONCHECKBOX_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONCHECKBOX_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonCheckbox.hpp"

class fuiRenderNode;

class FJ_ListButtonCheckbox_Normal : public FJ_ListButtonCheckbox
{
public:
    explicit FJ_ListButtonCheckbox_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonCheckbox* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
