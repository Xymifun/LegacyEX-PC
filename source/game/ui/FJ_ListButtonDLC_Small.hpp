#ifndef GAME_UI_FJ_LISTBUTTONDLC_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONDLC_SMALL_HPP

#include "game/ui/FJ_ListButtonDLC.hpp"

class fuiRenderNode;

class FJ_ListButtonDLC_Small : public FJ_ListButtonDLC
{
public:
    explicit FJ_ListButtonDLC_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonDLC* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
