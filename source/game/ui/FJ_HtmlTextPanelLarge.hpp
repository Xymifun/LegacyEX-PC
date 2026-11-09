#ifndef GAME_UI_FJ_HTMLTEXTPANELLARGE_HPP
#define GAME_UI_FJ_HTMLTEXTPANELLARGE_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_HtmlTextPanelLarge : public FJ_FuiNode
{
private:
    fuiRenderNodeEditText* m_textField;
    FJ_FuiNode* m_pointerTextPanel;
    bool m_fixedWidthEnabled;
    int m_fixedWidth;

public:
    FJ_HtmlTextPanelLarge(fuiRenderNode* pRenderNode);

    void SetLabel(const std::wstring& label);
    static FJ_HtmlTextPanelLarge* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetFixedWidth(int width);
    std::wstring GetLabel();
};

#endif
