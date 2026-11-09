#include "game/ui/FJ_KeyboardButtons_Keys_Small.hpp"

FJ_KeyboardButtons_Keys_Small::FJ_KeyboardButtons_Keys_Small(fuiRenderNode* renderNode) : FJ_KeyboardButtons(renderNode)
{
    m_keyboardOffset = 2;
}

FJ_KeyboardButtons* FJ_KeyboardButtons_Keys_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_KeyboardButtons_Keys_Small(renderNode);
}

unsigned int FJ_KeyboardButtons_Keys_Small::getType()
{
    return 0x801d;
}
