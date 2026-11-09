#ifndef GAME_UI_FJ_TOUCHCONTROLFRONTEND_HPP
#define GAME_UI_FJ_TOUCHCONTROLFRONTEND_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_TouchControlFrontend : public FJ_FuiNode
{
public:
    FJ_TouchControlFrontend(fuiRenderNode* pRenderNode);

    static FJ_TouchControlFrontend* Create(fuiRenderNode* pRenderNode);


    void Init();

    virtual unsigned int getType();
};

#endif
