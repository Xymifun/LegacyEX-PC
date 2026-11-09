#ifndef GAME_UI_FJ_LABEL_SMALLGREEN_HPP
#define GAME_UI_FJ_LABEL_SMALLGREEN_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SmallGreen : public FJ_Label
{
public:
    FJ_Label_SmallGreen(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}
    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
