#include "game/ui/FJ_FurnaceArrow_Small.hpp"

FJ_FurnaceArrow_Small::FJ_FurnaceArrow_Small(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    initFiller();
}

void FJ_FurnaceArrow_Small::InitBarType()
{
    m_barType = true;
    m_reverse = false;
}

FJ_FurnaceArrow_Small* FJ_FurnaceArrow_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_FurnaceArrow_Small(renderNode);
}

uint32_t FJ_FurnaceArrow_Small::getType()
{
    return 0x8050;
}
