#include "game/ui/FJ_ToolTip_Small.hpp"

FJ_ToolTip_Small::FJ_ToolTip_Small(fuiRenderNode* renderNode) : FJ_ToolTip(renderNode)
{
}

FJ_ToolTip* FJ_ToolTip_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ToolTip_Small(renderNode);
}

unsigned int FJ_ToolTip_Small::getType()
{
    return 0x8001;
}
