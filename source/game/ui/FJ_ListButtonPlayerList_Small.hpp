#ifndef GAME_UI_FJ_LISTBUTTONPLAYERLIST_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONPLAYERLIST_SMALL_HPP

#include "game/ui/FJ_ListButtonPlayerList.hpp"

class fuiRenderNode;

class FJ_ListButtonPlayerList_Small : public FJ_ListButtonPlayerList
{
public:
    explicit FJ_ListButtonPlayerList_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonPlayerList* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
