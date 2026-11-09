#ifndef GAME_UI_FJ_HTMLTEXT_HPP
#define GAME_UI_FJ_HTMLTEXT_HPP

#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class FJ_ScrollArrow;
class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_HtmlText : public FJ_Base
{
public:
    bool m_bUseScrollRect;
    bool m_bNoBackground;
    int m_iFocusDirection;
    float m_fScrollSpeed;
    fuiRenderNodeEditText* m_pText;
    float m_fScrollLeft;
    float m_fWidth;
    float m_fScrollTop;
    float m_fScrollBottom;
    FJ_FuiNode* m_pUpArrow;
    FJ_ScrollArrow* m_pDownArrow;
    bool m_bScrollingDown;
    bool m_bScrollingUp;
    bool m_bStopAtTarget;
    int m_iTargetScroll;
    int m_iCharacterHeight;
    int m_iScrollDelay;
    int m_iFlickDelay;
    unsigned char m_bScrollingEnabled;
    unsigned char m_bInitialized;
    int m_iRealWidth;
    int m_iRealHeight;
    void (*m_pScrollingCompleteDelegate)();
    bool m_bTouchScrolling;
    int m_iTouchStart;
    int m_iTouchPosition;
    int m_iTouchScroll;
    int m_iTouchTarget;

    FJ_HtmlText(fuiRenderNode* renderNode);
    virtual ~FJ_HtmlText();
    static FJ_FuiNode* Create(fuiRenderNode* renderNode);
    bool CanScroll(bool up);
    void HandleScrollListArrows(bool positionArrows);
    void UpdateScrolling(FJ_Event* event);
    static void _UpdateScrolling(FJ_FuiNode* node, FJ_Event* event);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void GetScrollTarget(bool page);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    int getTextHeight();
    virtual unsigned int getType();
    virtual void setVisible(bool visible);
    void UpdateScrollListArrows();
    virtual void setX(float x);
    virtual void setY(float y);
    virtual void setHeight(float height);
    virtual void Init(const std::wstring& label);
    void GetScrollDistance();
    virtual void SetLabel(const std::wstring& label);
    int GetNumberOfLines();
    int GetOptimalHeight(int lines);
    float GetRealWidth();
    float GetRealHeight();
    void TouchScroll(int position, bool active);
    void SetScrollingCompleteDelegate(void (*delegate)());
    std::wstring GetLabel();
    void SetScrollingEnabled(bool enabled);
};

#endif
