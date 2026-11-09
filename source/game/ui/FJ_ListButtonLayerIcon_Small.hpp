#ifndef GAME_UI_FJ_LISTBUTTONLAYERICON_SMALL_HPP
#define GAME_UI_FJ_LISTBUTTONLAYERICON_SMALL_HPP

#include "game/ui/FJ_ListButtonLayerIcon.hpp"

class fuiRenderNode;

class FJ_ListButtonLayerIcon_Small : public FJ_ListButtonLayerIcon
{
public:
    explicit FJ_ListButtonLayerIcon_Small(fuiRenderNode* renderNode);
    static FJ_ListButtonLayerIcon* Create(fuiRenderNode* renderNode);
    virtual uint32_t getType();
};

#endif
