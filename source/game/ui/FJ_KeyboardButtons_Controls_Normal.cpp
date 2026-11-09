#include "game/ui/FJ_KeyboardButtons_Controls_Normal.hpp"

FJ_KeyboardButtons_Controls_Normal::FJ_KeyboardButtons_Controls_Normal(fuiRenderNode* pRenderNode)
    : FJ_KeyboardButtons(pRenderNode)
{
    m_keyboardOffset = 3;
}

FJ_KeyboardButtons* FJ_KeyboardButtons_Controls_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_KeyboardButtons_Controls_Normal(pRenderNode);
}

uint32_t FJ_KeyboardButtons_Controls_Normal::getType()
{
    return 0x801a;
}
