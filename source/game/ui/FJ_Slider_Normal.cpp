#include "game/ui/FJ_Slider_Normal.hpp"

FJ_Slider_Normal::FJ_Slider_Normal(fuiRenderNode* renderNode) : FJ_Slider(renderNode)
{
}

FJ_Slider* FJ_Slider_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_Slider_Normal(renderNode);
}

uint32_t FJ_Slider_Normal::getType()
{
    return 0x8007;
}
