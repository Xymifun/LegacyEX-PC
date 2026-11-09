#ifndef GAME_UI_FJ_LABEL_SKINSELECTCENTER_HPP
#define GAME_UI_FJ_LABEL_SKINSELECTCENTER_HPP

#include "game/ui/FJ_Label.hpp"

class fuiRenderNode;

class FJ_Label_SkinSelectCenter : public FJ_Label
{
public:
    FJ_Label_SkinSelectCenter(fuiRenderNode* pRenderNode) : FJ_Label(pRenderNode) {}


    static FJ_Label* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
