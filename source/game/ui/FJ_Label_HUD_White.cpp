#include "game/ui/FJ_Label_HUD_White.hpp"

FJ_Label* FJ_Label_HUD_White::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_White(renderNode);
}

unsigned int FJ_Label_HUD_White::getType()
{
    return 0x8076;
}

void FJ_Label_HUD_White::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
