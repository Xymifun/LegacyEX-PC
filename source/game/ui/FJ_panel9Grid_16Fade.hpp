#ifndef GAME_UI_FJ_PANEL9GRID_16FADE_HPP
#define GAME_UI_FJ_PANEL9GRID_16FADE_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_panel9Grid_16Fade : public FJ_FuiNode
{
public:
    FJ_panel9Grid_16Fade(fuiRenderNode* renderNode);
    static FJ_FuiNode* Create(fuiRenderNode* renderNode);
    void ChangeState(int state);
    virtual unsigned int getType();
};

#endif
