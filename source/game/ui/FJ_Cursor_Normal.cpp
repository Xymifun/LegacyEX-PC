#include "game/ui/FJ_Cursor_Normal.hpp"

FJ_Cursor_Normal::FJ_Cursor_Normal(fuiRenderNode* pRenderNode) : FJ_Cursor(pRenderNode)
{
}

FJ_Cursor* FJ_Cursor_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Cursor_Normal(pRenderNode);
}

unsigned int FJ_Cursor_Normal::getType()
{
    return 93;
}
