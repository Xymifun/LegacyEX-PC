#include "game/ui/FJ_Label_DragonHealth.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_DragonHealth::Create(fuiRenderNode* renderNode)
{
    FJ_Label* label = new FJ_Label_DragonHealth(renderNode);
    return label;
}

unsigned int FJ_Label_DragonHealth::getType()
{
    return 0x8066;
}
