#ifndef GAME_UI_FJ_LISTBUTTONICONLEFT_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONICONLEFT_SMALL_HPP

#include "game/ui/FJ_ListButtonIconLeft.hpp"

class fuiRenderNode;

class FJ_ListButtonIconLeft_Small : public FJ_ListButtonIconLeft
{
public:
    explicit FJ_ListButtonIconLeft_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonIconLeft* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
