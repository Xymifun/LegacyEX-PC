#include "game/ui/FJ_TextInput_Normal.hpp"

FJ_TextInput_Normal::FJ_TextInput_Normal(fuiRenderNode* renderNode) : FJ_TextInput(renderNode)
{
}

FJ_TextInput* FJ_TextInput_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_TextInput_Normal(renderNode);
}

uint32_t FJ_TextInput_Normal::getType()
{
    return 0x803f;
}
