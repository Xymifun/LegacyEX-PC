#ifndef GAME_UI_FJ_HTMLTEXT_WHITESMALL_HPP
#define GAME_UI_FJ_HTMLTEXT_WHITESMALL_HPP

#include "game/ui/FJ_HtmlText.hpp"

class fuiRenderNode;

class FJ_HtmlText_WhiteSmall : public FJ_HtmlText
{
public:
    FJ_HtmlText_WhiteSmall(fuiRenderNode* pRenderNode) : FJ_HtmlText(pRenderNode) {}

    static FJ_HtmlText* Create(fuiRenderNode* pRenderNode);
    virtual void ASConstructor();
    virtual unsigned int getType();
};

#endif
