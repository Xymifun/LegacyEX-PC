#include "game/ui/FJ_DragonHealth_04.hpp"

FJ_DragonHealth_04::FJ_DragonHealth_04(fuiRenderNode* renderNode) : FJ_DragonHealth(renderNode)
{
}

FJ_DragonHealth* FJ_DragonHealth_04::Create(fuiRenderNode* renderNode)
{
    return new FJ_DragonHealth_04(renderNode);
}

uint32_t FJ_DragonHealth_04::getType()
{
    return 0x8045;
}
