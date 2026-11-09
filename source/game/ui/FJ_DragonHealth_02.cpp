#include "game/ui/FJ_DragonHealth_02.hpp"

FJ_DragonHealth_02::FJ_DragonHealth_02(fuiRenderNode* renderNode) : FJ_DragonHealth(renderNode)
{
}

FJ_DragonHealth* FJ_DragonHealth_02::Create(fuiRenderNode* renderNode)
{
    return new FJ_DragonHealth_02(renderNode);
}

uint32_t FJ_DragonHealth_02::getType()
{
    return 0x8043;
}
