#ifndef GAME_UI_FJ_FURNACEFLAME_HPP
#define GAME_UI_FJ_FURNACEFLAME_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_FurnaceFlame : public FJ_ProgressBar
{
public:
    explicit FJ_FurnaceFlame(fuiRenderNode* renderNode);
    virtual void InitBarType();
    static FJ_FurnaceFlame* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
