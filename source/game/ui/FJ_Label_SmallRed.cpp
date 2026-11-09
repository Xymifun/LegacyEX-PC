#include "game/ui/FJ_Label_SmallRed.hpp"

FJ_Label* FJ_Label_SmallRed::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SmallRed(renderNode);
}

unsigned int FJ_Label_SmallRed::getType()
{
    return 0x8083;
}
