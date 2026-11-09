#include "game/ui/FJ_BrewingArrow_Small.hpp"

FJ_BrewingArrow_Small::FJ_BrewingArrow_Small(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_BrewingArrow_Small::InitBarType()
{
    m_barType = false;
    m_reverse = false;
}

FJ_BrewingArrow_Small* FJ_BrewingArrow_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingArrow_Small(renderNode);
}

unsigned int FJ_BrewingArrow_Small::getType()
{
    return 0x804a;
}

void FJ_BrewingArrow_Small::setProgress(float progress)
{
    if (progress != 0.0f)
        progress = 1.0f - progress;

    FJ_ProgressBar::setProgress(progress);
}
