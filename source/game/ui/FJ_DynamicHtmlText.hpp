#ifndef GAME_UI_FJ_DYNAMICHTMLTEXT_HPP
#define GAME_UI_FJ_DYNAMICHTMLTEXT_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Event;
class FJ_Label;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_DynamicHtmlText : public FJ_FuiNode
{
public:
    int m_iTopMargin;
    int m_iLabelSpacing;
    float m_fScrollSpeed;
    FJ_Label* m_pLabels[20];
    FJ_FuiNode* m_pContentContainer;
    std::vector<std::wstring> m_text;
    std::vector<FJ_Label*> m_labels;
    std::vector<int> m_labelTextIndices;
    int m_iCharacterHeight;
    int m_iContentHeight;
    int m_iTargetScrollPosition;
    fuiRect m_scrollRect;
    FJ_ScrollArrow* m_pUpArrow;
    FJ_ScrollArrow* m_pDownArrow;
    int m_iScrollDelay;
    int m_iFlickDelay;
    FJ_FuiNode* m_pHtmlTextPanel;
    float m_fRealWidth;
    float m_fRealHeight;
    bool m_bListReady;
    bool m_bAtEnd;
    int m_iScrollDirection;
    int m_iTargetDirection;
    bool m_bKeyboardScroll;
    int m_iTouchScroll;
    bool m_bTouchScrolling;
    bool m_bTouchFlick;
    int m_iCurrentTouchPosition;
    int m_iTouchStart;
    int m_iFlickOffset;

    FJ_DynamicHtmlText(fuiRenderNode* pRenderNode);
    virtual ~FJ_DynamicHtmlText();
    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    int getActiveLabelHeight() const;
    void UpdateTouchFlickScrolling();
    unsigned int SetNextLabel(int index, bool append);
    void HandleScrollListArrows(bool positionArrows);
    void UpdateScrolling(FJ_Event* event);
    static void _UpdateScrolling(FJ_FuiNode* node, FJ_Event* event);
    void Init();
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void ASConstructor();
    virtual unsigned int getType();
    virtual void setVisible(bool visible);
    void Reload(float width, float height);
    void SetupList();
    void AddText(const std::wstring& text, bool setup);
    float GetRealWidth();
    float GetRealHeight();
    void TouchScroll(int offset, bool active);
};

#endif
