#ifndef GAME_UI_FJ_CHECKPOINTMARKER_HPP
#define GAME_UI_FJ_CHECKPOINTMARKER_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_CheckpointMarker : public FJ_FuiNode
{
public:
    FJ_CheckpointMarker(fuiRenderNode* pRenderNode);
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
