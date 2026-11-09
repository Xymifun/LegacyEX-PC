#include "game/ui/FJ_ListButtonScoreBoard_Normal.hpp"

FJ_ListButtonScoreBoard_Normal::FJ_ListButtonScoreBoard_Normal(fuiRenderNode* renderNode)
    : FJ_ListButtonScoreBoard(renderNode)
{
    m_small = false;
}

FJ_ListButtonScoreBoard* FJ_ListButtonScoreBoard_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonScoreBoard_Normal(renderNode);
}

uint32_t FJ_ListButtonScoreBoard_Normal::getType()
{
    return 0x802a;
}
