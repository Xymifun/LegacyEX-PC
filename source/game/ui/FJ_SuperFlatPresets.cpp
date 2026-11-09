#include "game/ui/FJ_SuperFlatPresets.hpp"

FJ_SuperFlatPresets::FJ_SuperFlatPresets(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_SuperFlatPresets::Create(fuiRenderNode* renderNode)
{
    return new FJ_SuperFlatPresets(renderNode);
}

unsigned int FJ_SuperFlatPresets::getType()
{
    return 40;
}
