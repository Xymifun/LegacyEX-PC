#include "game/ui/FJ_FurnaceFlame_Small.hpp"

FJ_FurnaceFlame_Small::FJ_FurnaceFlame_Small(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_FurnaceFlame_Small::InitBarType()
{
    m_barType = false;
    m_reverse = true;
}

FJ_FurnaceFlame_Small* FJ_FurnaceFlame_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_FurnaceFlame_Small(renderNode);
}

uint32_t FJ_FurnaceFlame_Small::getType()
{
    return 0x8052;
}
