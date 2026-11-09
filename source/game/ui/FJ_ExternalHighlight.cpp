#include "game/ui/FJ_ExternalHighlight.hpp"

FJ_ExternalHighlight::FJ_ExternalHighlight(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

FJ_ExternalHighlight* FJ_ExternalHighlight::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ExternalHighlight(pRenderNode);
}

unsigned int FJ_ExternalHighlight::getType()
{
    return 19;
}
