#include "game/ui/FJ_Label_HUD_Level_01.hpp"

FJ_Label_HUD_Level_01::~FJ_Label_HUD_Level_01()
{
}

FJ_LabelOutline* FJ_Label_HUD_Level_01::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_Level_01(renderNode);
}

unsigned int FJ_Label_HUD_Level_01::getType()
{
    return 0x8097;
}
