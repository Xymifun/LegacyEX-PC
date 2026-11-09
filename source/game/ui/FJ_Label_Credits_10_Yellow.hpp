#ifndef GAME_UI_FJ_LABEL_CREDITS_10_YELLOW_HPP
#define GAME_UI_FJ_LABEL_CREDITS_10_YELLOW_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_Credits_10_Yellow : public FJ_LabelOutline
{
public:
    FJ_Label_Credits_10_Yellow(fuiRenderNode* pRenderNode) : FJ_LabelOutline(pRenderNode) {}
    virtual ~FJ_Label_Credits_10_Yellow();

    static FJ_LabelOutline* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
