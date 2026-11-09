#include "game/ui/FJ_ListButtonAchievement_Normal.hpp"

FJ_ListButtonAchievement_Normal::FJ_ListButtonAchievement_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonAchievement(renderNode)
{
}

FJ_ListButtonAchievement* FJ_ListButtonAchievement_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonAchievement_Normal(renderNode);
}

uint32_t FJ_ListButtonAchievement_Normal::getType()
{
    return 0x800e;
}
