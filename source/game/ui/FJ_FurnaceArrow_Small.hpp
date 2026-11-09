#ifndef GAME_UI_FJ_FURNACEARROW_SMALL_HPP
#define GAME_UI_FJ_FURNACEARROW_SMALL_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_FurnaceArrow_Small : public FJ_ProgressBar
{
public:
    FJ_FurnaceArrow_Small(fuiRenderNode* renderNode);
    static FJ_FurnaceArrow_Small* Create(fuiRenderNode* renderNode);
    virtual void InitBarType();
    virtual uint32_t getType();
};

#endif
