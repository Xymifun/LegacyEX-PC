#include "game/ui/FJ_Label_VerySmallWhite.hpp"

FJ_Label* FJ_Label_VerySmallWhite::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_VerySmallWhite(renderNode);
}

unsigned int FJ_Label_VerySmallWhite::getType()
{
    return 0x8087;
}
