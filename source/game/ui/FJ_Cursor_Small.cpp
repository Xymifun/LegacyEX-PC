#include "game/ui/FJ_Cursor_Small.hpp"

FJ_Cursor_Small::FJ_Cursor_Small(fuiRenderNode* pRenderNode) : FJ_Cursor(pRenderNode)
{
}

FJ_Cursor* FJ_Cursor_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Cursor_Small(pRenderNode);
}

unsigned int FJ_Cursor_Small::getType()
{
    return 0x5d;
}
