#ifndef GAME_UI_FJ_LABEL_HUD_LEVEL_01_HPP
#define GAME_UI_FJ_LABEL_HUD_LEVEL_01_HPP

#include "game/ui/FJ_LabelOutline.hpp"

class fuiRenderNode;

class FJ_Label_HUD_Level_01 : public FJ_LabelOutline
{
public:
    FJ_Label_HUD_Level_01(fuiRenderNode* renderNode) : FJ_LabelOutline(renderNode) {}

    virtual ~FJ_Label_HUD_Level_01();
    static FJ_LabelOutline* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
