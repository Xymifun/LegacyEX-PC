#include "game/ui/FJ_Slider_Small.hpp"

FJ_Slider_Small::FJ_Slider_Small(fuiRenderNode* renderNode) : FJ_Slider(renderNode)
{
}

FJ_Slider* FJ_Slider_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_Slider_Small(renderNode);
}

unsigned int FJ_Slider_Small::getType()
{
    return 0x8008;
}
