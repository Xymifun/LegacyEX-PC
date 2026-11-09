#ifndef GAME_UI_FJ_LABEL_DRAGONHEALTHVERYSMALL_HPP
#define GAME_UI_FJ_LABEL_DRAGONHEALTHVERYSMALL_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_DragonHealthVerySmall : public FJ_Label
{
public:
    FJ_Label_DragonHealthVerySmall(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
