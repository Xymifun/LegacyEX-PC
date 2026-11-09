#ifndef GAME_UI_FJ_DYNAMICHTMLTEXT_SMALL_HPP
#define GAME_UI_FJ_DYNAMICHTMLTEXT_SMALL_HPP

#include "game/ui/FJ_DynamicHtmlText.hpp"

class fuiRenderNode;

class FJ_DynamicHtmlText_Small : public FJ_DynamicHtmlText
{
public:
    FJ_DynamicHtmlText_Small(fuiRenderNode* pRenderNode) : FJ_DynamicHtmlText(pRenderNode) {}
    static FJ_DynamicHtmlText* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
