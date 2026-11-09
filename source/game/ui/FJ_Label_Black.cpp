#include "game/ui/FJ_Label_Black.hpp"

FJ_Label* FJ_Label_Black::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Black(renderNode);
}

unsigned int FJ_Label_Black::getType()
{
    return 0x805d;
}
