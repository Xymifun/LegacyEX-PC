#ifndef GAME_UI_FJ_KILLBAR_HPP
#define GAME_UI_FJ_KILLBAR_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_KillBar : public FJ_ProgressBar
{
public:
    explicit FJ_KillBar(fuiRenderNode* renderNode);
    static FJ_ProgressBar* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    virtual void Init(const std::wstring& label);
    virtual void SetLabel(const std::wstring& label);
};

#endif
