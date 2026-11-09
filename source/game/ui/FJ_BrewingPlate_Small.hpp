#ifndef GAME_UI_FJ_BREWINGPLATE_SMALL_HPP
#define GAME_UI_FJ_BREWINGPLATE_SMALL_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class FJ_BrewingPlate_Small : public FJ_ProgressBar
{
public:
    explicit FJ_BrewingPlate_Small(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_BrewingPlate_Small* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
