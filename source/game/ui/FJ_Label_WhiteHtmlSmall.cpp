#include "game/ui/FJ_Label_WhiteHtmlSmall.hpp"

FJ_Label* FJ_Label_WhiteHtmlSmall::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_WhiteHtmlSmall(renderNode);
}

unsigned int FJ_Label_WhiteHtmlSmall::getType()
{
    return 0x808a;
}

void FJ_Label_WhiteHtmlSmall::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
