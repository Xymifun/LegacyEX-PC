#include "game/ui/FJ_Label_Red.hpp"

FJ_Label* FJ_Label_Red::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Red(renderNode);
}

unsigned int FJ_Label_Red::getType()
{
    return 0x8077;
}
