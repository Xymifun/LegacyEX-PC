#ifndef GAME_UI_FJ_LISTBUTTONACHIEVEMENT_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONACHIEVEMENT_NORMAL_HPP

#include <stdint.h>

#include "game/ui/FJ_ListButtonAchievement.hpp"

class fuiRenderNode;

class FJ_ListButtonAchievement_Normal : public FJ_ListButtonAchievement
{
public:
    FJ_ListButtonAchievement_Normal(fuiRenderNode* renderNode);
    static FJ_ListButtonAchievement* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
