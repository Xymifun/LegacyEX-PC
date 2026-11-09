#ifndef GAME_UI_FJ_SLOTLIST_HPP
#define GAME_UI_FJ_SLOTLIST_HPP

#include <stdint.h>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_SlotList_Slot;
class fuiRenderNode;

class FJ_SlotList : public FJ_FuiNode
{
    friend class FJ_CraftingMenu;
    friend class FJ_TradingMenu;
    friend class fuiRenderNodeTimeline;

private:
    int m_slotSize;
    bool m_slotVisible;
    int m_rows;
    int m_columns;
    std::vector<FJ_SlotList_Slot*> m_slots;
    bool m_loaded;

public:
    FJ_SlotList(fuiRenderNode* pRenderNode);

    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    float getWidth();
    float getHeight();
    void Init();
    void Clear();
    void addSlot(int slot, bool highlighted, bool visible);
    bool SetSlotRedBox(int slot, bool show);
    bool SetSlotHighlight(int slot, bool highlighted);
    virtual void setVisible(bool visible);
    int getRows() { return m_rows; }
    int getColumns() { return m_columns; }
    bool getLoaded() { return m_loaded; }
    void ShowSlot(int slot, bool visible);
};

#endif
