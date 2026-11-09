#ifndef GAME_UI_FJ_FURNACEARROW_HPP
#define GAME_UI_FJ_FURNACEARROW_HPP

#include "game/ui/FJ_ProgressBar.hpp"

class fuiRenderNode;

class FJ_FurnaceArrow : public FJ_ProgressBar
{
public:
    explicit FJ_FurnaceArrow(fuiRenderNode* renderNode);
    static FJ_FurnaceArrow* Create(fuiRenderNode* renderNode);
    virtual void InitBarType();
    virtual uint32_t getType();
};

#endif
