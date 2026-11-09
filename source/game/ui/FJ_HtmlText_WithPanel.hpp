#ifndef GAME_UI_FJ_HTMLTEXT_WITHPANEL_HPP
#define GAME_UI_FJ_HTMLTEXT_WITHPANEL_HPP

#include <string>

#include "game/ui/FJ_HtmlText.hpp"

class FJ_FuiNode;
class fuiRenderNode;

class FJ_HtmlText_WithPanel : public FJ_HtmlText
{
    friend class fuiRenderNodeTimeline;

public:
    FJ_HtmlText_WithPanel(fuiRenderNode* renderNode);
    virtual ~FJ_HtmlText_WithPanel();
    static FJ_HtmlText_WithPanel* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void setVisible(bool visible);
    virtual void setY(float y);
    virtual void Init(const std::wstring& label);

private:
    int m_panelOffset;
    FJ_FuiNode* m_panelNode;
};

#endif
