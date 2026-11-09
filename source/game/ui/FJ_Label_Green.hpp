#ifndef GAME_UI_FJ_LABEL_GREEN_HPP
#define GAME_UI_FJ_LABEL_GREEN_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_Green : public FJ_Label
{
public:
    FJ_Label_Green(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
