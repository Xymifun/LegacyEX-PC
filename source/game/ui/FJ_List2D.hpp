#ifndef GAME_UI_FJ_LIST2D_HPP
#define GAME_UI_FJ_LIST2D_HPP

#include <stdint.h>
#include <vector>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_List2D : public FJ_Base
{
    friend class FJ_AchievementsMenu;

protected:
    typedef void (*FJ_ScrollDelegateType)(FJ_FuiNode*);

    bool m_outline;
    int m_itemSpacing;
    std::vector<std::vector<FJ_FuiNode*> > m_items;
    int m_activeItem;
    FJ_ScrollArrow* m_upArrow;
    FJ_ScrollArrow* m_downArrow;
    int m_visibleRows;
    int m_itemsPerRow;
    int m_activeRow;
    int m_activeColumn;
    int m_firstVisibleRow;
    int m_listHeight;
    int m_touchY;
    int m_touchX;
    int m_touchRow;
    bool m_touchFocus;
    FJ_ScrollDelegateType m_scrollDelegate;
    FJ_FuiNode* m_scrollDelegateNode;

public:
    explicit FJ_List2D(fuiRenderNode* renderNode);
    virtual ~FJ_List2D() {}
    void HighlightItem(int index);
    void HandlePageUpDownNavigation(bool down);
    FJ_FuiNode* getActiveItem();
    void SetListFocus();
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void Init(int id);
    void UpdateScrollListArrows();
    virtual void setX(float x);
    virtual void setY(float y);
    void SetScrollDelegate(void (*delegate)(FJ_FuiNode*), FJ_FuiNode* node);
    void HandleScrollListArrows(bool addToStage);
    void ScrollList(bool up, bool initial, bool immediate);
    void ItemFocusInHandler(FJ_Event* event);
    static void _ItemFocusInHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void SetNewObjectWidth(int width);
    void addNewListItem(FJ_Base* item, int id);
    void removeAllItems();
    void EnableItem(int index, bool enabled);
    void SetTouchFocus(int x, int y, bool focus);
    uint32_t CanTouchFocus(int x, int y);
    uint32_t CanTouchTrigger(int x, int y);
    void setAction(unsigned char action, char* value);
    void LoseListFocus();
    uint32_t getiActiveItem();
    uint32_t getiNumButtons();
    void focusOutHandler(FJ_Event* event);
    int GetListHeight();
    FJ_FuiNode* ReturnItem(int index);
};

#endif
