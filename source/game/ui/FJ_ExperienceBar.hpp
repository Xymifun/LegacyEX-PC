#ifndef GAME_UI_FJ_EXPERIENCEBAR_HPP
#define GAME_UI_FJ_EXPERIENCEBAR_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_ExperienceBar : public FJ_ProgressBar
{
public:
    explicit FJ_ExperienceBar(fuiRenderNode* renderNode);

    virtual void GetBarFiller();
    static FJ_ExperienceBar* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    virtual void Init(const std::wstring& label);
};

#endif
