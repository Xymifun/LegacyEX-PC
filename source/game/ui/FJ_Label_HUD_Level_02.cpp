#include "game/ui/FJ_Label_HUD_Level_02.hpp"

FJ_Label_HUD_Level_02::~FJ_Label_HUD_Level_02()
{
}

FJ_LabelOutline* FJ_Label_HUD_Level_02::Create(fuiRenderNode* renderNode)
{
    FJ_LabelOutline* outline = new FJ_Label_HUD_Level_02(renderNode);
    return outline;
}

unsigned int FJ_Label_HUD_Level_02::getType()
{
    return 0x8098;
}
