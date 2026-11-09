#include "game/ui/FJ_KillBarSmall.hpp"

FJ_KillBarSmall::FJ_KillBarSmall(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    FJ_ProgressBar::InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

FJ_ProgressBar* FJ_KillBarSmall::Create(fuiRenderNode* renderNode)
{
    return new FJ_KillBarSmall(renderNode);
}

uint32_t FJ_KillBarSmall::getType()
{
    return 0x8054;
}

void FJ_KillBarSmall::Init(const std::wstring& label)
{
    FJ_Base::SetLabelAlignment(1);
    FJ_Base::InitBase(label, -1);
}

void FJ_KillBarSmall::SetLabel(const std::wstring& label)
{
    m_textContainerVisible = true;
    m_textContainerChanged = true;
    m_manualWrap = false;
    m_htmlText = true;
    m_textOnly = false;
    FJ_Base::SetLabel(label);
}
