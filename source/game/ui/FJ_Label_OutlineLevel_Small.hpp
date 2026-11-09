#ifndef GAME_UI_FJ_LABEL_OUTLINELEVEL_SMALL_HPP
#define GAME_UI_FJ_LABEL_OUTLINELEVEL_SMALL_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_OutlineLevel_Small : public FJ_LabelOutline
{
public:
    FJ_Label_OutlineLevel_Small(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}

    virtual ~FJ_Label_OutlineLevel_Small();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
