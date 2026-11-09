#ifndef GAME_UI_FJ_TEXTINPUT_HPP
#define GAME_UI_FJ_TEXTINPUT_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_DebugCreateSchematic;
class FJ_DebugFillArea;
class FJ_FuiNode;
class FJ_Factory;
class FJ_ScrollArrowH;
class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_TextInput : public FJ_Base
{
public:
    explicit FJ_TextInput(fuiRenderNode* renderNode);
    virtual ~FJ_TextInput() {}

    static FJ_Base* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void ChangeState(int state);
    void PositionScrollArrows();
    void HandleScrollArrows(bool addArrows);
    void UpdateScrollPosition(float caretPosition, bool movingRight, bool force);
    void SetCaretIndex(int index, bool movingRight, bool force);
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void MoveCaretLeft();
    void MoveCaretRight();
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void Init(std::wstring name, int id);
    virtual void setX(float x);
    virtual void setY(float y);
    virtual void setVisible(bool visible);
    virtual void SetLabel(const std::wstring& label);
    void SetCharLimit(int limit);
    void HandleDirectInput(unsigned int character);
    virtual void HandleCharInput(unsigned int character);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    virtual void SetNewObjectWidth(int width);
    void NonFocusSelect();

private:
    friend class FJ_DebugCreateSchematic;
    friend class FJ_DebugSetCamera;
    friend class FJ_DebugFillArea;
    friend class fuiRenderNodeTimeline;

    int m_scrollWidth;
    int m_state;
    bool m_firstFocus;
    int m_caretIndex;
    float m_scrollOffset;
    int m_charLimit;
    FJ_FuiNode* m_pCaret;
    FJ_FuiNode* m_pOutline;
    FJ_FuiNode* m_pLeftArrow;
    FJ_FuiNode* m_pRightArrow;
    fuiRenderNodeEditText* m_pTextField;
    bool m_bKeepCaret;
    bool m_bUseScale;
};

#endif
