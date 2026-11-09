#ifndef GAME_UI_FJ_LISTBUTTONACHIEVEMENT_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONACHIEVEMENT_SMALL_HPP

#include "game/ui/FJ_ListButtonAchievement.hpp"

class fuiRenderNode;

class FJ_ListButtonAchievement_Small : public FJ_ListButtonAchievement
{
public:
    FJ_ListButtonAchievement_Small(fuiRenderNode* pRenderNode);
    static FJ_ListButtonAchievement* Create(fuiRenderNode* pRenderNode);
    virtual uint32_t getType();
};

#endif
