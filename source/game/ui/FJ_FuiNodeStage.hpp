#ifndef GAME_UI_FJ_FUINODESTAGE_HPP
#define GAME_UI_FJ_FUINODESTAGE_HPP

#include "game/ui/FJ_FuiNode.hpp"

class FJ_FuiNodeStage : public FJ_FuiNode
{
private:
    FJ_FuiNode* m_pFocus;

public:
    FJ_FuiNodeStage(fuiRenderNode* pRenderNode);

    void removeFocus(FJ_FuiNode* pNode);
    void setFocus(FJ_FuiNode* pNode);
    FJ_FuiNode* getFocus();
    float getStageHeight();
    float getStageWidth();
};

#endif
