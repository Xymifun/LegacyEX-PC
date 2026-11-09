#include "game/ui/FJ_TextInput_Small.hpp"

FJ_TextInput_Small::FJ_TextInput_Small(fuiRenderNode* renderNode) : FJ_TextInput(renderNode)
{
}

FJ_TextInput* FJ_TextInput_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_TextInput_Small(renderNode);
}

unsigned int FJ_TextInput_Small::getType()
{
    return 0x803e;
}
