#include "game/ui/FJ_Label_SmallGrey.hpp"

FJ_Label* FJ_Label_SmallGrey::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SmallGrey(renderNode);
}

unsigned int FJ_Label_SmallGrey::getType()
{
    return 0x8082;
}
