#ifndef GAME_UI_FJ_EXTERNALHIGHLIGHT_HPP
#define GAME_UI_FJ_EXTERNALHIGHLIGHT_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_ExternalHighlight : public FJ_FuiNode
{
public:
    FJ_ExternalHighlight(fuiRenderNode* pRenderNode);

    static FJ_ExternalHighlight* Create(fuiRenderNode* pRenderNode);


    virtual unsigned int getType();
};

#endif
