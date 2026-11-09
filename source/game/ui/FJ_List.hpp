#ifndef GAME_UI_FJ_LIST_HPP
#define GAME_UI_FJ_LIST_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNodeStage;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_List : public FJ_Base
{
public:
    explicit FJ_List(fuiRenderNode* renderNode);
    virtual ~FJ_List();

    uint32_t FindHighlightItem(int index, bool backward, bool wrap);
    void HighlightItem(int index, bool fromTouch);
    void HandlePageUpDownNavigation(bool up);
    void LoseListFocus();
    int getINumButtons();
    void SetListFocus();
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void Init(int id);
    void UpdateScrollListArrows();
    virtual void setX(float x);
    virtual void setY(float y);
    void ListKeyDownHandler(FJ_Event* event);
    static void _ListKeyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    float GetListItemHeight(int index);
    void GetMaxItemsVisibleUp();
    std::wstring GetItemLabel(int index);
    void HandleScrollListArrows(bool addArrows);
    void ScrollList(bool down, bool page, bool fromTouch);
    void ItemFocusInHandler(FJ_Event* event);
    static void _ItemFocusInHandler(FJ_FuiNode* node, FJ_Event* event);
    int GetListHeight();
    void UpdateTouchSize();
    int GetListWidth();
    virtual void SetNewObjectWidth(int width);
    void addNewListItem(FJ_Base* item, const std::wstring& label, int id);
    void removeAllItems();
    void SetButtonLabel(int index, const std::wstring& label);
    void SetItemAlpha(int index, float alpha);
    void SetItemLabel(int index, const std::wstring& label);
    void EnableItem(int index, bool enabled);
    void SetTouchFocus(int x, int y, bool focus);
#ifdef MINECRAFT_PC
    void PcPressSlider(int x, int y);
    void PcHoverItem(int y);
#endif
    uint32_t CanTouchFocus(int x, int y);
    void BlockTouchTrigger(bool block);
    uint32_t CanTouchTrigger(int x, int y);
    void FixNavigation(FJ_FuiNode* node);
    void RemoveItem(int index);
    FJ_FuiNode* ReturnItem(int index);
    int GetActiveItem();
    uint32_t GetTopItem();
    void setAction(unsigned char action, char* value);
    uint32_t getIActiveItem();
    void SetVerticalSpacing(int spacing);
    int GetVerticalSpacing();
    void MoveItem(int index, bool down);

protected:
    friend class FJ_GameModeScoreboard;

    bool m_checkboxTick;
    int m_verticalSpacing;
    std::string m_action;
    bool m_initialized;
    std::vector<FJ_FuiNode*> m_items;
    int m_activeItem;
    int m_topItem;
    FJ_ScrollArrow* m_upArrow;
    FJ_ScrollArrow* m_downArrow;
    FJ_FuiNode* m_background;
    int m_maxItemsVisibleUp;
    int m_touchSize;
    bool m_keyDown;
    bool m_keyUp;
    int m_touchFocusY;
    int m_touchY;
    int m_touchItem;
    bool m_scrollChanged;
    bool m_touchActive;
    bool m_blockTouchTrigger;
    bool m_highlightFromTouch;
#ifdef MINECRAFT_PC
    bool m_pcHovering;
#endif
};

#endif
