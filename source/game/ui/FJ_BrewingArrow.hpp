#ifndef GAME_UI_FJ_BREWINGARROW_HPP
#define GAME_UI_FJ_BREWINGARROW_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_BrewingArrow : public FJ_ProgressBar
{
public:
    explicit FJ_BrewingArrow(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_BrewingArrow* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
    virtual void setProgress(float progress);
};

#endif
