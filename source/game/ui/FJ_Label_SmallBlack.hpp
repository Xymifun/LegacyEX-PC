#ifndef GAME_UI_FJ_LABEL_SMALLBLACK_HPP
#define GAME_UI_FJ_LABEL_SMALLBLACK_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SmallBlack : public FJ_Label
{
public:
    FJ_Label_SmallBlack(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}
    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
