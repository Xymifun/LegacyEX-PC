#include "game/ui/FJ_ToolTip_Normal.hpp"

FJ_ToolTip_Normal::FJ_ToolTip_Normal(fuiRenderNode* renderNode) : FJ_ToolTip(renderNode)
{
}

FJ_ToolTip* FJ_ToolTip_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ToolTip_Normal(renderNode);
}

unsigned int FJ_ToolTip_Normal::getType()
{
    return 0x8002;
}
