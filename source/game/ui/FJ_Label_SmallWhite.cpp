#include "game/ui/FJ_Label_SmallWhite.hpp"

FJ_Label* FJ_Label_SmallWhite::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_SmallWhite(renderNode);
}

unsigned int FJ_Label_SmallWhite::getType()
{
    return 0x8084;
}
