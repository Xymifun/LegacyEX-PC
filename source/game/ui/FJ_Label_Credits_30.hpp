#ifndef GAME_UI_FJ_LABEL_CREDITS_30_HPP
#define GAME_UI_FJ_LABEL_CREDITS_30_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_Credits_30 : public FJ_LabelOutline
{
public:
    FJ_Label_Credits_30(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}
    virtual ~FJ_Label_Credits_30();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
