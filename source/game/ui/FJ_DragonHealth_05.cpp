#include "game/ui/FJ_DragonHealth_05.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_DragonHealth_05::FJ_DragonHealth_05(fuiRenderNode* renderNode) : FJ_DragonHealth(renderNode)
{
}

FJ_DragonHealth* FJ_DragonHealth_05::Create(fuiRenderNode* renderNode)
{
    return new FJ_DragonHealth_05(renderNode);
}

unsigned int FJ_DragonHealth_05::getType()
{
    return 0x8046;
}
