#include "game/ui/FJ_BrewingArrow.hpp"

FJ_BrewingArrow::FJ_BrewingArrow(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_BrewingArrow::InitBarType()
{
    m_barType = false;
    m_reverse = false;
}

FJ_BrewingArrow* FJ_BrewingArrow::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingArrow(renderNode);
}

uint32_t FJ_BrewingArrow::getType()
{
    return 0x8049;
}

void FJ_BrewingArrow::setProgress(float progress)
{
    if (progress != 0.0f)
        progress = 1.0f - progress;
    FJ_ProgressBar::setProgress(progress);
}
