#include "game/ui/FJ_Label_Html.hpp"

FJ_Label* FJ_Label_Html::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_Html(renderNode);
}

unsigned int FJ_Label_Html::getType()
{
    return 0x806c;
}

void FJ_Label_Html::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
