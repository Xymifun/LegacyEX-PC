#ifndef GAME_UI_FJ_LABEL_SKINSELECTCENTER_SMALL_HPP
#define GAME_UI_FJ_LABEL_SKINSELECTCENTER_SMALL_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SkinSelectCenter_Small : public FJ_Label
{
public:
    FJ_Label_SkinSelectCenter_Small(fuiRenderNode* pRenderNode) : FJ_Label(pRenderNode) {}

    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
