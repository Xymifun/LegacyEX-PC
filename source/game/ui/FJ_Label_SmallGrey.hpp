#ifndef GAME_UI_FJ_LABEL_SMALLGREY_HPP
#define GAME_UI_FJ_LABEL_SMALLGREY_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SmallGrey : public FJ_Label
{
public:
    explicit FJ_Label_SmallGrey(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}


    static FJ_Label* Create(fuiRenderNode* renderNode);

    virtual unsigned int getType();
};

#endif
