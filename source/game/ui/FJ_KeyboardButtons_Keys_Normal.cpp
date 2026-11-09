#include "game/ui/FJ_KeyboardButtons_Keys_Normal.hpp"

FJ_KeyboardButtons_Keys_Normal::FJ_KeyboardButtons_Keys_Normal(fuiRenderNode* renderNode)
    : FJ_KeyboardButtons(renderNode)
{
    m_keyboardOffset = 3;
}

FJ_KeyboardButtons* FJ_KeyboardButtons_Keys_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_KeyboardButtons_Keys_Normal(renderNode);
}

uint32_t FJ_KeyboardButtons_Keys_Normal::getType()
{
    return 0x801c;
}
