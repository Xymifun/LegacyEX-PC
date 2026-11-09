#ifndef GAME_UI_FJ_FURNACEFLAME_SMALL_HPP
#define GAME_UI_FJ_FURNACEFLAME_SMALL_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_FurnaceFlame_Small : public FJ_ProgressBar
{
public:
    explicit FJ_FurnaceFlame_Small(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_FurnaceFlame_Small* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
