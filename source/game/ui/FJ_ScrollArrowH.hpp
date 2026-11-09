#ifndef GAME_UI_FJ_SCROLLARROWH_HPP
#define GAME_UI_FJ_SCROLLARROWH_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_ScrollArrowH : public FJ_FuiNode
{
public:
    int m_iState;

    FJ_ScrollArrowH(fuiRenderNode* pRenderNode);
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void ChangeState(int iState);
};

#endif
