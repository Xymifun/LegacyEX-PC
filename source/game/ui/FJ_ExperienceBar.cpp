#include "game/ui/FJ_ExperienceBar.hpp"

#include <string>

FJ_ExperienceBar::FJ_ExperienceBar(fuiRenderNode* renderNode) : FJ_ProgressBar(renderNode)
{
    GetBarFiller();
    FJ_ProgressBar::initFiller();
}

FJ_ExperienceBar* FJ_ExperienceBar::Create(fuiRenderNode* renderNode)
{
    return new FJ_ExperienceBar(renderNode);
}

void FJ_ExperienceBar::GetBarFiller()
{
    m_pFiller = getChildByName(std::string("ExpBarFiller"))->asMovieClip();
}

uint32_t FJ_ExperienceBar::getType()
{
    return 0x8048;
}

void FJ_ExperienceBar::Init(const std::wstring& label)
{
    (void)label;
}
