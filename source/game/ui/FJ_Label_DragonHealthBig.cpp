#include "game/ui/FJ_Label_DragonHealthBig.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_DragonHealthBig::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_DragonHealthBig(renderNode);
}

unsigned int FJ_Label_DragonHealthBig::getType()
{
    return 0x8067;
}
