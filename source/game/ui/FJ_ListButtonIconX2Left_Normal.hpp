#ifndef GAME_UI_FJ_LISTBUTTONICONX2LEFT_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONICONX2LEFT_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonIconLeft.hpp"

class fuiRenderNode;

class FJ_ListButtonIconX2Left_Normal : public FJ_ListButtonIconLeft
{
public:
    explicit FJ_ListButtonIconX2Left_Normal(fuiRenderNode* renderNode);

    static FJ_ListButtonIconLeft* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
