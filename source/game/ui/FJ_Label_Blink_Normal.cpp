#include "game/ui/FJ_Label_Blink_Normal.hpp"

FJ_Label_Blink_Normal::FJ_Label_Blink_Normal(fuiRenderNode* renderNode) : FJ_Label_Blink(renderNode)
{
}

FJ_Label_Blink* FJ_Label_Blink_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Blink_Normal(renderNode);
}

unsigned int FJ_Label_Blink_Normal::getType()
{
    return 0x8005;
}
