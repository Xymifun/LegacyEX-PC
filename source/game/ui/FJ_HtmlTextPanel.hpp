#ifndef GAME_UI_FJ_HTMLTEXTPANEL_HPP
#define GAME_UI_FJ_HTMLTEXTPANEL_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_HtmlTextPanel : public FJ_FuiNode
{
private:
    fuiRenderNodeEditText* m_textField;
    FJ_FuiNode* m_pointerTextPanel;
    bool m_fixedWidthEnabled;
    int m_fixedWidth;

public:
    FJ_HtmlTextPanel(fuiRenderNode* pRenderNode);

    void SetLabel(const std::wstring& label);
    static FJ_HtmlTextPanel* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetFixedWidth(int width);
    std::wstring GetLabel();
};

#endif
