#include "game/ui/FJ_Label_White_Centered.hpp"

FJ_Label* FJ_Label_White_Centered::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_White_Centered(renderNode);
}

unsigned int FJ_Label_White_Centered::getType()
{
    return 0x808b;
}
