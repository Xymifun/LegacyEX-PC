#ifndef GAME_UI_FJ_BREWINGARROW_SMALL_HPP
#define GAME_UI_FJ_BREWINGARROW_SMALL_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_BrewingArrow_Small : public FJ_ProgressBar
{
public:
    explicit FJ_BrewingArrow_Small(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_BrewingArrow_Small* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void setProgress(float progress);
};

#endif
