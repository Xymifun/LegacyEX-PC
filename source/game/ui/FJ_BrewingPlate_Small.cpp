#include "game/ui/FJ_BrewingPlate_Small.hpp"

FJ_BrewingPlate_Small::FJ_BrewingPlate_Small(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_BrewingPlate_Small::InitBarType()
{
    m_barType = true;
    m_reverse = false;
}

FJ_BrewingPlate_Small* FJ_BrewingPlate_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_BrewingPlate_Small(renderNode);
}

unsigned int FJ_BrewingPlate_Small::getType()
{
    return 0x804e;
}
