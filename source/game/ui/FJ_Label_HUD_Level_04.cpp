#include "game/ui/FJ_Label_HUD_Level_04.hpp"

FJ_Label_HUD_Level_04::~FJ_Label_HUD_Level_04()
{
}

FJ_LabelOutline* FJ_Label_HUD_Level_04::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_Level_04(renderNode);
}

unsigned int FJ_Label_HUD_Level_04::getType()
{
    return 0x809a;
}
