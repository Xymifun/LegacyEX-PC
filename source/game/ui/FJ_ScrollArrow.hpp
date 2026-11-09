#ifndef GAME_UI_FJ_SCROLLARROW_HPP
#define GAME_UI_FJ_SCROLLARROW_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_ScrollArrow : public FJ_FuiNode
{
public:
    int m_iState;

    FJ_ScrollArrow(fuiRenderNode* pRenderNode);
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    void ChangeState(int iState);
    virtual unsigned int getType();
};

#endif
