#include "game/ui/FJ_BrewingPlate.hpp"

FJ_BrewingPlate::FJ_BrewingPlate(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    initFiller();
}

void FJ_BrewingPlate::InitBarType()
{
    m_barType = true;
    m_reverse = false;
}

FJ_BrewingPlate* FJ_BrewingPlate::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingPlate(renderNode);
}

uint32_t FJ_BrewingPlate::getType()
{
    return 0x804d;
}
