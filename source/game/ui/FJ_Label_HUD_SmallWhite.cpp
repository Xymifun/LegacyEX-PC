#include "game/ui/FJ_Label_HUD_SmallWhite.hpp"

FJ_Label* FJ_Label_HUD_SmallWhite::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_SmallWhite(renderNode);
}

unsigned int FJ_Label_HUD_SmallWhite::getType()
{
    return 0x8070;
}

void FJ_Label_HUD_SmallWhite::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
