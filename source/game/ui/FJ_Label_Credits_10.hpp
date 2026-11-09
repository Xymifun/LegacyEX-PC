#ifndef GAME_UI_FJ_LABEL_CREDITS_10_HPP
#define GAME_UI_FJ_LABEL_CREDITS_10_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_Credits_10 : public FJ_LabelOutline
{
public:
    FJ_Label_Credits_10(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}

    virtual ~FJ_Label_Credits_10();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
