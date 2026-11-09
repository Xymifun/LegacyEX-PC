#ifndef GAME_UI_FJ_LABEL_OUTLINETITLE_HPP
#define GAME_UI_FJ_LABEL_OUTLINETITLE_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_OutlineTitle : public FJ_LabelOutline
{
public:
    FJ_Label_OutlineTitle(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}

    virtual ~FJ_Label_OutlineTitle();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
