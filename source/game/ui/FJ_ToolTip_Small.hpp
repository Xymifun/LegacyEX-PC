#ifndef GAME_UI_FJ_TOOLTIP_SMALL_HPP
#define GAME_UI_FJ_TOOLTIP_SMALL_HPP

#include "game/ui/FJ_ToolTip.hpp"

class fuiRenderNode;

class FJ_ToolTip_Small : public FJ_ToolTip
{
public:
    explicit FJ_ToolTip_Small(fuiRenderNode* renderNode);
    static FJ_ToolTip* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
};

#endif
