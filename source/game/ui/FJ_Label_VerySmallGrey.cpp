#include "game/ui/FJ_Label_VerySmallGrey.hpp"

FJ_Label* FJ_Label_VerySmallGrey::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_VerySmallGrey(renderNode);
}

unsigned int FJ_Label_VerySmallGrey::getType()
{
    return 0x8086;
}
