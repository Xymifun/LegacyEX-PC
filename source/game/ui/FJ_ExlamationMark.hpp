#ifndef GAME_UI_FJ_EXLAMATIONMARK_HPP
#define GAME_UI_FJ_EXLAMATIONMARK_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_ExlamationMark : public FJ_FuiNode
{
public:
    FJ_ExlamationMark(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode) {}

    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);

    virtual unsigned int getType();
};

#endif
