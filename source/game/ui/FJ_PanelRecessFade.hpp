#ifndef GAME_UI_FJ_PANELRECESSFADE_HPP
#define GAME_UI_FJ_PANELRECESSFADE_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_PanelRecessFade : public FJ_FuiNode
{
public:
    FJ_PanelRecessFade(fuiRenderNode* pRenderNode);
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    void ChangeState(int iState);
    virtual unsigned int getType();
};

#endif
