#include "game/ui/FJ_Label_HtmlEndPoem.hpp"

FJ_Label* FJ_Label_HtmlEndPoem::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HtmlEndPoem(renderNode);
}

unsigned int FJ_Label_HtmlEndPoem::getType()
{
    return 0x806d;
}

void FJ_Label_HtmlEndPoem::SetLabel(const std::wstring& label)
{
    m_htmlText = true;
    FJ_Label::SetLabel(label);
}
