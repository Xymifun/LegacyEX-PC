#ifndef GAME_UI_FJ_HTMLTEXT_NORMAL_HPP
#define GAME_UI_FJ_HTMLTEXT_NORMAL_HPP

#include "game/ui/FJ_HtmlText.hpp"

class fuiRenderNode;

class FJ_HtmlText_Normal : public FJ_HtmlText
{
private:
public:
    FJ_HtmlText_Normal(fuiRenderNode* pRenderNode) : FJ_HtmlText(pRenderNode) {}
    static FJ_HtmlText* Create(fuiRenderNode* pRenderNode);
    virtual void ASConstructor();
    virtual unsigned int getType();
};

#endif
