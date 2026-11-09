#include "game/ui/FJ_Label_Blink_Small.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label_Blink_Small::FJ_Label_Blink_Small(fuiRenderNode* renderNode) : FJ_Label_Blink(renderNode)
{
}

FJ_Label_Blink* FJ_Label_Blink_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Blink_Small(renderNode);
}

unsigned int FJ_Label_Blink_Small::getType()
{
    return 0x8006;
}
