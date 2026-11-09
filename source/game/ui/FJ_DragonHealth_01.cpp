#include "game/ui/FJ_DragonHealth_01.hpp"

FJ_DragonHealth_01::FJ_DragonHealth_01(fuiRenderNode* renderNode) : FJ_DragonHealth(renderNode)
{
}

FJ_DragonHealth* FJ_DragonHealth_01::Create(fuiRenderNode* renderNode)
{
    return new FJ_DragonHealth_01(renderNode);
}

uint32_t FJ_DragonHealth_01::getType()
{
    return 0x8042;
}
