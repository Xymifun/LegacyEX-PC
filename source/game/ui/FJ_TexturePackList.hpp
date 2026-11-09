#ifndef GAME_UI_FJ_TEXTUREPACKLIST_HPP
#define GAME_UI_FJ_TEXTUREPACKLIST_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_ScrollArrowH;
class fuiRenderNode;

class FJ_TexturePackList : public FJ_Base
{
    friend class fuiRenderNodeTimeline;

public:
    explicit FJ_TexturePackList(fuiRenderNode* renderNode);

    void ChangeState(int state);
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void HandleScrollListArrows(bool addArrows);
    void ScrollList(bool down, bool page, bool fromTouch);
    void GotoSelectedSlot();
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void NavigateList(bool down);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void UpdateLabel();
    void HandleUpdateLabel(FJ_Event* event);
    static void _HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event);
    static FJ_TexturePackList* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void Init(const std::wstring& name, int id);
    void SelectSlot(int slot);
    void EnableSelector(bool enabled);
    void addPack(int id, const std::wstring& name);
    void removeAllItems();
    void SetTouchFocus(int x, int y, bool focus);
    unsigned int CanTouchTrigger(int x, int y);
    float GetRealHeight();
    int getSelectedSlot();

private:
    float m_disabledAlpha;
    int m_state;
    std::vector<FJ_FuiNode*> m_items;
    FJ_FuiNode* m_slotArea;
    int m_topSlot;
    int m_selectedSlot;
    int m_focusSlot;
    int m_visibleSlots;
    int m_slotSpacing;
    FJ_ScrollArrowH* m_leftArrow;
    FJ_ScrollArrowH* m_rightArrow;
    bool m_selectorEnabled;
    int m_touchX;
    int m_touchSlot;
    bool m_touchMoved;
};

#endif
