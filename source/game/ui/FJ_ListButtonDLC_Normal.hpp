#ifndef GAME_UI_FJ_LISTBUTTONDLC_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONDLC_NORMAL_HPP

#include "game/ui/FJ_ListButtonDLC.hpp"

class fuiRenderNode;

class FJ_ListButtonDLC_Normal : public FJ_ListButtonDLC
{
public:
    explicit FJ_ListButtonDLC_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonDLC* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
