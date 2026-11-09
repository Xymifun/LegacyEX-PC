#include "game/ui/FJ_Label_HUD_Level_03.hpp"

FJ_Label_HUD_Level_03::~FJ_Label_HUD_Level_03()
{
}

FJ_LabelOutline* FJ_Label_HUD_Level_03::Create(fuiRenderNode* renderNode)
{
    FJ_LabelOutline* outline = new FJ_Label_HUD_Level_03(renderNode);
    return outline;
}

unsigned int FJ_Label_HUD_Level_03::getType()
{
    return 0x8099;
}
