#include "game/ui/FJ_KeyboardButtons_Controls_Small.hpp"

FJ_KeyboardButtons_Controls_Small::FJ_KeyboardButtons_Controls_Small(fuiRenderNode* renderNode)
    : FJ_KeyboardButtons(renderNode)
{
    m_keyboardOffset = 2;
}

FJ_KeyboardButtons* FJ_KeyboardButtons_Controls_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_KeyboardButtons_Controls_Small(renderNode);
}

uint32_t FJ_KeyboardButtons_Controls_Small::getType()
{
    return 0x801b;
}
