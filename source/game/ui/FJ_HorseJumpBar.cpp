#include "game/ui/FJ_HorseJumpBar.hpp"

#include <string>

FJ_HorseJumpBar::FJ_HorseJumpBar(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    GetBarFiller();
    initFiller();
}

FJ_HorseJumpBar* FJ_HorseJumpBar::Create(fuiRenderNode* renderNode)
{
    return new FJ_HorseJumpBar(renderNode);
}

void FJ_HorseJumpBar::GetBarFiller()
{
    m_pFiller = getChildByName("HorseBarFiller")->asMovieClip();
}

uint32_t FJ_HorseJumpBar::getType()
{
    return 0x8047;
}

void FJ_HorseJumpBar::Init(const std::wstring& label)
{
}
