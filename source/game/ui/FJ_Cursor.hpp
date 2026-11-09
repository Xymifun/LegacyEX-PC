#ifndef GAME_UI_FJ_CURSOR_HPP
#define GAME_UI_FJ_CURSOR_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Cursor : public FJ_FuiNode
{
private:
    fuiRenderNodeEditText* m_textField;
    FJ_FuiNode* m_textPanel;
    FJ_FuiNode* m_pointerTextPanel;
    FJ_FuiNode* m_cursorImage;
    int m_topAlignment;

public:
    int m_panelOffsetX;

    FJ_Cursor(fuiRenderNode* pRenderNode);
    virtual ~FJ_Cursor() {}
    virtual void ASConstructor();

    void TopAlignPanel();
    void AlignPanel(bool bRight);
    void UpdatePanelAlignment();
    void SetLabel(const std::wstring& label);
};

#endif
