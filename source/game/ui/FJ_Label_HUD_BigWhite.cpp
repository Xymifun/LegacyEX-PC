#include "game/ui/FJ_Label_HUD_BigWhite.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_HUD_BigWhite::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_Label_HUD_BigWhite(pRenderNode);
}

unsigned int FJ_Label_HUD_BigWhite::getType()
{
    return 0x8071;
}

void FJ_Label_HUD_BigWhite::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
