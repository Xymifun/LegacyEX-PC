#ifndef GAME_UI_FJ_DYNAMICHTMLTEXT_WITHPANEL_NORMAL_HPP
#define GAME_UI_FJ_DYNAMICHTMLTEXT_WITHPANEL_NORMAL_HPP

#include "game/ui/FJ_DynamicHtmlText.hpp"

class fuiRenderNode;

class FJ_DynamicHtmlText_WithPanel_Normal : public FJ_DynamicHtmlText
{
public:
    FJ_DynamicHtmlText_WithPanel_Normal(fuiRenderNode* pRenderNode) : FJ_DynamicHtmlText(pRenderNode) {}

    static FJ_DynamicHtmlText* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
