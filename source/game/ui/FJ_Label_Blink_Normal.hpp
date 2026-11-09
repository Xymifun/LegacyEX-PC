#ifndef GAME_UI_FJ_LABEL_BLINK_NORMAL_HPP
#define GAME_UI_FJ_LABEL_BLINK_NORMAL_HPP

#include "game/ui/FJ_Label_Blink.hpp"

class fuiRenderNode;

class FJ_Label_Blink_Normal : public FJ_Label_Blink
{
public:
    explicit FJ_Label_Blink_Normal(fuiRenderNode* renderNode);
    static FJ_Label_Blink* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
