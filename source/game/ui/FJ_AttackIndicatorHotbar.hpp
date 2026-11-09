#ifndef GAME_UI_FJ_ATTACKINDICATORHOTBAR_HPP
#define GAME_UI_FJ_ATTACKINDICATORHOTBAR_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_AttackIndicatorHotbar : public FJ_ProgressBar
{
public:
    explicit FJ_AttackIndicatorHotbar(fuiRenderNode* renderNode);
    static FJ_AttackIndicatorHotbar* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void InitBarType();
};

#endif
