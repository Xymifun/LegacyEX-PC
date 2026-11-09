#ifndef GAME_UI_FJ_TOUCHCONTROL_HPP
#define GAME_UI_FJ_TOUCHCONTROL_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_TouchControl : public FJ_FuiNode
{
public:
    FJ_TouchControl(fuiRenderNode* pRenderNode);

    void Init();
    static FJ_TouchControl* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
