#ifndef GAME_UI_FJ_LABEL_SKINSELECT_OFF_HPP
#define GAME_UI_FJ_LABEL_SKINSELECT_OFF_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SkinSelect_Off : public FJ_Label
{
public:
    FJ_Label_SkinSelect_Off(fuiRenderNode* pRenderNode) : FJ_Label(pRenderNode) {}


    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
