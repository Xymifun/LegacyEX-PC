#include "game/ui/FJ_KillBar.hpp"

FJ_KillBar::FJ_KillBar(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    GetBarFiller();
    initFiller();
}

FJ_ProgressBar* FJ_KillBar::Create(fuiRenderNode* renderNode)
{
    return new FJ_KillBar(renderNode);
}

uint32_t FJ_KillBar::getType()
{
    return 0x8053;
}

void FJ_KillBar::Init(const std::wstring& label)
{
    SetLabelAlignment(1);
    InitBase(label, -1);
}

void FJ_KillBar::SetLabel(const std::wstring& label)
{
    m_textContainerVisible = true;
    m_textContainerChanged = true;
    m_manualWrap = false;
    m_htmlText = true;
    m_textOnly = false;
    FJ_Base::SetLabel(label);
}
