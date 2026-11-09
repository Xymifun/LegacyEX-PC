#include "game/ui/FJ_TextInput_Vita.hpp"

FJ_TextInput_Vita::FJ_TextInput_Vita(fuiRenderNode* renderNode) : FJ_TextInput(renderNode)
{
}

FJ_TextInput* FJ_TextInput_Vita::Create(fuiRenderNode* renderNode)
{
    return new FJ_TextInput_Vita(renderNode);
}

unsigned int FJ_TextInput_Vita::getType()
{
    return 0x8040;
}
