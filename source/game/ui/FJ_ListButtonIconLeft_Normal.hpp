#ifndef GAME_UI_FJ_LISTBUTTONICONLEFT_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONICONLEFT_NORMAL_HPP

#include "game/ui/FJ_ListButtonIconLeft.hpp"

class fuiRenderNode;

class FJ_ListButtonIconLeft_Normal : public FJ_ListButtonIconLeft
{
public:
    explicit FJ_ListButtonIconLeft_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonIconLeft* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
