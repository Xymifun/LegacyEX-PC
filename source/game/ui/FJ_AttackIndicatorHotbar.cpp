#include "game/ui/FJ_AttackIndicatorHotbar.hpp"

FJ_AttackIndicatorHotbar::FJ_AttackIndicatorHotbar(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_AttackIndicatorHotbar::InitBarType()
{
    m_barType = false;
    m_reverse = true;
}

FJ_AttackIndicatorHotbar* FJ_AttackIndicatorHotbar::Create(fuiRenderNode* renderNode)
{
    return new FJ_AttackIndicatorHotbar(renderNode);
}

unsigned int FJ_AttackIndicatorHotbar::getType()
{
    return 0x8057;
}
