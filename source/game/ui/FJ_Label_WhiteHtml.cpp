#include "game/ui/FJ_Label_WhiteHtml.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_Label* FJ_Label_WhiteHtml::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_WhiteHtml(renderNode);
}

unsigned int FJ_Label_WhiteHtml::getType()
{
    return 0x8089;
}

void FJ_Label_WhiteHtml::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
