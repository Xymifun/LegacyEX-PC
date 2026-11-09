#ifndef GAME_UI_FJ_PANEL9GRIDFADE_HPP
#define GAME_UI_FJ_PANEL9GRIDFADE_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_Panel9GridFade : public FJ_FuiNode
{
public:
    FJ_Panel9GridFade(fuiRenderNode* pRenderNode);
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    void ChangeState(int iState);
    virtual unsigned int getType();
};

#endif
