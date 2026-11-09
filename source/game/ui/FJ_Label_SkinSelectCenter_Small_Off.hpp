#ifndef GAME_UI_FJ_LABEL_SKINSELECTCENTER_SMALL_OFF_HPP
#define GAME_UI_FJ_LABEL_SKINSELECTCENTER_SMALL_OFF_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SkinSelectCenter_Small_Off : public FJ_Label
{
public:
    FJ_Label_SkinSelectCenter_Small_Off(fuiRenderNode* renderNode) : FJ_Label(renderNode) {}

    static FJ_Label* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
