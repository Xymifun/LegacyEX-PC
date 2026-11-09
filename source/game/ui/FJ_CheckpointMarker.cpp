#include "game/ui/FJ_CheckpointMarker.hpp"

FJ_CheckpointMarker::FJ_CheckpointMarker(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

FJ_FuiNode* FJ_CheckpointMarker::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_CheckpointMarker(pRenderNode);
}

unsigned int FJ_CheckpointMarker::getType()
{
    return 0x14;
}
