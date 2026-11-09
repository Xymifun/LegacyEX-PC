#include "game/ui/FJ_AttackIndicatorCrosshair.hpp"

FJ_AttackIndicatorCrosshair::FJ_AttackIndicatorCrosshair(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

FJ_AttackIndicatorCrosshair* FJ_AttackIndicatorCrosshair::Create(fuiRenderNode* renderNode)
{
    return new FJ_AttackIndicatorCrosshair(renderNode);
}

void FJ_AttackIndicatorCrosshair::InitBarType()
{
    m_barType = true;
    m_reverse = false;
}

uint32_t FJ_AttackIndicatorCrosshair::getType()
{
    return 0x8056;
}
