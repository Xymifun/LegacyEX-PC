#include "game/ui/FJ_Label_HtmlEndPoemHD.hpp"

FJ_Label* FJ_Label_HtmlEndPoemHD::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HtmlEndPoemHD(renderNode);
}

unsigned int FJ_Label_HtmlEndPoemHD::getType()
{
    return 0x806e;
}

void FJ_Label_HtmlEndPoemHD::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
