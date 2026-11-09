#include "game/ui/FJ_FurnaceArrow.hpp"

FJ_FurnaceArrow::FJ_FurnaceArrow(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    InitBarType();
    FJ_ProgressBar::GetBarFiller();
    FJ_ProgressBar::initFiller();
}

void FJ_FurnaceArrow::InitBarType()
{
    m_barType = true;
    m_reverse = false;
}

FJ_FurnaceArrow* FJ_FurnaceArrow::Create(fuiRenderNode* renderNode)
{
    return new FJ_FurnaceArrow(renderNode);
}

uint32_t FJ_FurnaceArrow::getType()
{
    return 0x804f;
}
