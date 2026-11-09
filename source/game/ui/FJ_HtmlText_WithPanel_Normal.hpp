#ifndef GAME_UI_FJ_HTMLTEXT_WITHPANEL_NORMAL_HPP
#define GAME_UI_FJ_HTMLTEXT_WITHPANEL_NORMAL_HPP

#include "game/ui/FJ_HtmlText_WithPanel.hpp"

class fuiRenderNode;

class FJ_HtmlText_WithPanel_Normal : public FJ_HtmlText_WithPanel
{
public:
    FJ_HtmlText_WithPanel_Normal(fuiRenderNode* pRenderNode) : FJ_HtmlText_WithPanel(pRenderNode) {}
    static FJ_HtmlText_WithPanel* Create(fuiRenderNode* pRenderNode);
    virtual void ASConstructor();
    virtual unsigned int getType();
};

#endif
