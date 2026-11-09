#ifndef GAME_UI_FJ_LABEL_VERYSMALLGREY_HPP
#define GAME_UI_FJ_LABEL_VERYSMALLGREY_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_VerySmallGrey : public FJ_Label
{
public:
    explicit FJ_Label_VerySmallGrey(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);

    virtual unsigned int getType();
};

#endif
