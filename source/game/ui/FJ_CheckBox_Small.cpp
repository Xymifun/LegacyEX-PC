#include "game/ui/FJ_CheckBox_Small.hpp"

FJ_CheckBox_Small::FJ_CheckBox_Small(fuiRenderNode* pRenderNode) : FJ_CheckBox(pRenderNode)
{
}

FJ_CheckBox* FJ_CheckBox_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_CheckBox_Small(pRenderNode);
}

unsigned int FJ_CheckBox_Small::getType()
{
    return 0x8004;
}
