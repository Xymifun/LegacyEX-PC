#ifndef GAME_UI_FJ_BREWINGBUBBLES_HPP
#define GAME_UI_FJ_BREWINGBUBBLES_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_BrewingBubbles : public FJ_ProgressBar
{
public:
    FJ_BrewingBubbles(fuiRenderNode* renderNode);
    static FJ_BrewingBubbles* Create(fuiRenderNode* renderNode);
    virtual void InitBarType();
    virtual uint32_t getType();
};

#endif
