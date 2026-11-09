#include "game/ui/FJ_CheckBox_Normal.hpp"

FJ_CheckBox_Normal::FJ_CheckBox_Normal(fuiRenderNode* renderNode) : FJ_CheckBox(renderNode)
{
}

FJ_CheckBox* FJ_CheckBox_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_CheckBox_Normal(renderNode);
}

uint32_t FJ_CheckBox_Normal::getType()
{
    return 0x8003;
}
