#include "game/ui/FJ_Label_HtmlSmall.hpp"

FJ_Label* FJ_Label_HtmlSmall::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HtmlSmall(renderNode);
}

unsigned int FJ_Label_HtmlSmall::getType()
{
    return 0x806f;
}

void FJ_Label_HtmlSmall::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
