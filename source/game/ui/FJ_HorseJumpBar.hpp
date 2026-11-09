#ifndef GAME_UI_FJ_HORSEJUMPBAR_HPP
#define GAME_UI_FJ_HORSEJUMPBAR_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_HorseJumpBar : public FJ_ProgressBar
{
public:
    explicit FJ_HorseJumpBar(fuiRenderNode* renderNode);
    virtual void GetBarFiller();
    static FJ_HorseJumpBar* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    virtual void Init(const std::wstring& label);
};

#endif
