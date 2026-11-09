#ifndef GAME_UI_FJ_LABEL_CREDITS_20_HPP
#define GAME_UI_FJ_LABEL_CREDITS_20_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_Credits_20 : public FJ_LabelOutline
{
public:
    FJ_Label_Credits_20(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}

    virtual ~FJ_Label_Credits_20();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
