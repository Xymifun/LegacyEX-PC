#include "game/ui/FJ_Label_DragonHealthVerySmall.hpp"

FJ_Label* FJ_Label_DragonHealthVerySmall::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_DragonHealthVerySmall(renderNode);
}

unsigned int FJ_Label_DragonHealthVerySmall::getType()
{
    return 0x8069;
}
