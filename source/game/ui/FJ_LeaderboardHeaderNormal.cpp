#include "game/ui/FJ_LeaderboardHeaderNormal.hpp"

FJ_LeaderboardHeaderNormal::FJ_LeaderboardHeaderNormal(fuiRenderNode* pRenderNode) : FJ_LeaderboardHeader(pRenderNode)
{
}

FJ_LeaderboardHeader* FJ_LeaderboardHeaderNormal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_LeaderboardHeaderNormal(pRenderNode);
}

unsigned int FJ_LeaderboardHeaderNormal::getType()
{
    return 0x6a;
}
