#ifndef GAME_UI_FJ_LISTLABEL_BLACK_HPP
#define GAME_UI_FJ_LISTLABEL_BLACK_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_ListLabel_Black : public FJ_Label
{
public:
    FJ_ListLabel_Black(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);

    virtual unsigned int getType();
};

#endif
