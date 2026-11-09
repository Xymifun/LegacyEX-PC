#include "game/ui/FJ_Label_HUD_VerySmallWhite.hpp"

FJ_Label* FJ_Label_HUD_VerySmallWhite::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_HUD_VerySmallWhite(pRenderNode);
}

unsigned int FJ_Label_HUD_VerySmallWhite::getType()
{
    return 0x8075;
}

void FJ_Label_HUD_VerySmallWhite::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
