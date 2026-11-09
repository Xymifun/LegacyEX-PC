#ifndef GAME_UI_FJ_BREWINGPLATE_HPP
#define GAME_UI_FJ_BREWINGPLATE_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_BrewingPlate : public FJ_ProgressBar
{
public:
    FJ_BrewingPlate(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_BrewingPlate* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
