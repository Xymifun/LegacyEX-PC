#ifndef GAME_UI_FJ_HTMLTEXT_WHITENORMAL_HPP
#define GAME_UI_FJ_HTMLTEXT_WHITENORMAL_HPP

#include "game/ui/FJ_HtmlText.hpp"

class fuiRenderNode;

class FJ_HtmlText_WhiteNormal : public FJ_HtmlText
{
private:
public:
    FJ_HtmlText_WhiteNormal(fuiRenderNode* pRenderNode);
    static FJ_HtmlText* Create(fuiRenderNode* pRenderNode);
    virtual void ASConstructor();
    virtual unsigned int getType();
};

#endif
