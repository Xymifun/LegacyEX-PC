#include "game/ui/FJ_BrewingBubbles_Small.hpp"

FJ_BrewingBubbles_Small::FJ_BrewingBubbles_Small(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    initFiller();
}

void FJ_BrewingBubbles_Small::InitBarType()
{
    m_barType = false;
    m_reverse = true;
}

FJ_BrewingBubbles_Small* FJ_BrewingBubbles_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingBubbles_Small(renderNode);
}

uint32_t FJ_BrewingBubbles_Small::getType()
{
    return 0x804c;
}
