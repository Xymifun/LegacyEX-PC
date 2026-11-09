#include "game/ui/FJ_BrewingBubbles.hpp"

FJ_BrewingBubbles::FJ_BrewingBubbles(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    initFiller();
}

void FJ_BrewingBubbles::InitBarType()
{
    m_barType = false;
    m_reverse = true;
}

FJ_BrewingBubbles* FJ_BrewingBubbles::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingBubbles(renderNode);
}

uint32_t FJ_BrewingBubbles::getType()
{
    return 0x804c;
}
