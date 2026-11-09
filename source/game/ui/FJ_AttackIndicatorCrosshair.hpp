#ifndef GAME_UI_FJ_ATTACKINDICATORCROSSHAIR_HPP
#define GAME_UI_FJ_ATTACKINDICATORCROSSHAIR_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_AttackIndicatorCrosshair : public FJ_ProgressBar
{
public:
    explicit FJ_AttackIndicatorCrosshair(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_AttackIndicatorCrosshair* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
