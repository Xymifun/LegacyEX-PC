#include "game/ui/FJ_DebugOptions.hpp"

FJ_DebugOptions::FJ_DebugOptions(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_DebugOptions::Create(fuiRenderNode* renderNode)
{
    return new FJ_DebugOptions(renderNode);
}

unsigned int FJ_DebugOptions::getType()
{
    return 56;
}
