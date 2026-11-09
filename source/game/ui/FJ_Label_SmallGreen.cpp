#include "game/ui/FJ_Label_SmallGreen.hpp"

FJ_Label* FJ_Label_SmallGreen::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SmallGreen(renderNode);
}

unsigned int FJ_Label_SmallGreen::getType()
{
    return 0x8081;
}
