#include "game/ui/FJ_ListButtonScoreBoard_Small.hpp"

#include "game/ui/FJ_ListButtonScoreBoard.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_ListButtonScoreBoard_Small::FJ_ListButtonScoreBoard_Small(fuiRenderNode* renderNode)
    : FJ_ListButtonScoreBoard(renderNode)
{
    m_small = true;
}

FJ_ListButtonScoreBoard* FJ_ListButtonScoreBoard_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonScoreBoard_Small(renderNode);
}

uint32_t FJ_ListButtonScoreBoard_Small::getType()
{
    return 0x802b;
}
