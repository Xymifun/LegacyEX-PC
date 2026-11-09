#ifndef GAME_UI_FJ_LABEL_OUTLINELEVEL_HPP
#define GAME_UI_FJ_LABEL_OUTLINELEVEL_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_OutlineLevel : public FJ_LabelOutline
{
public:
    explicit FJ_Label_OutlineLevel(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}

    virtual ~FJ_Label_OutlineLevel();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
