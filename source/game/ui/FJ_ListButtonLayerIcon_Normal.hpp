#ifndef GAME_UI_FJ_LISTBUTTONLAYERICON_NORMAL_HPP
#define GAME_UI_FJ_LISTBUTTONLAYERICON_NORMAL_HPP

#include "game/ui/FJ_ListButtonLayerIcon.hpp"

class fuiRenderNode;

class FJ_ListButtonLayerIcon_Normal : public FJ_ListButtonLayerIcon
{
public:
    explicit FJ_ListButtonLayerIcon_Normal(fuiRenderNode* pRenderNode);
    static FJ_ListButtonLayerIcon* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
