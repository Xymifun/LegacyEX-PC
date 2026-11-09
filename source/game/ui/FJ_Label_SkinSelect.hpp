#ifndef GAME_UI_FJ_LABEL_SKINSELECT_HPP
#define GAME_UI_FJ_LABEL_SKINSELECT_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SkinSelect : public FJ_Label
{
public:
    FJ_Label_SkinSelect(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
