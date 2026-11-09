#include "game/ui/FJ_ListButtonAchievement_Small.hpp"

FJ_ListButtonAchievement_Small::FJ_ListButtonAchievement_Small(fuiRenderNode* pRenderNode)
    : FJ_ListButtonAchievement(pRenderNode)
{
}

FJ_ListButtonAchievement* FJ_ListButtonAchievement_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ListButtonAchievement_Small(pRenderNode);
}

uint32_t FJ_ListButtonAchievement_Small::getType()
{
    return 0x800f;
}
