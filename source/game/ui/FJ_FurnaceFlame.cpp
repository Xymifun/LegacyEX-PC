#include "game/ui/FJ_FurnaceFlame.hpp"

FJ_FurnaceFlame::FJ_FurnaceFlame(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_FurnaceFlame::InitBarType()
{
    m_barType = false;
    m_reverse = true;
}

FJ_FurnaceFlame* FJ_FurnaceFlame::Create(fuiRenderNode* renderNode)
{
    return new FJ_FurnaceFlame(renderNode);
}

uint32_t FJ_FurnaceFlame::getType()
{
    return 0x8051;
}
