#include "game/ui/FJ_Label_HUD_Level_05.hpp"

FJ_Label_HUD_Level_05::~FJ_Label_HUD_Level_05()
{
}

FJ_LabelOutline* FJ_Label_HUD_Level_05::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_Level_05(renderNode);
}

unsigned int FJ_Label_HUD_Level_05::getType()
{
    return 0x809b;
}
