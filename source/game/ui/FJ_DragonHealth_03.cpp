#include "game/ui/FJ_DragonHealth_03.hpp"

FJ_DragonHealth_03::FJ_DragonHealth_03(fuiRenderNode* renderNode) : FJ_DragonHealth(renderNode)
{
}

FJ_DragonHealth* FJ_DragonHealth_03::Create(fuiRenderNode* renderNode)
{
    return new FJ_DragonHealth_03(renderNode);
}

uint32_t FJ_DragonHealth_03::getType()
{
    return 0x8044;
}
