#ifndef GAME_UI_FJ_SLOTLIST_SLOT_HPP
#define GAME_UI_FJ_SLOTLIST_SLOT_HPP

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_SlotList_Slot : public FJ_FuiNode
{
private:
    int m_iSlotId;
    FJ_FuiNode* m_pRedBox;
    FJ_FuiNode* m_pSlotControl;
    FJ_FuiNode* m_pHighlight;
    bool m_bRedBoxActive;
    bool m_bHighlightActive;

public:
    FJ_SlotList_Slot(fuiRenderNode* pRenderNode);
    virtual ~FJ_SlotList_Slot();
    void SetParams(int iWidth, bool bVisible);
    void SetSlotID(int iSlotId);
    void SetHighlightActive(bool bActive);
    void SetRedBoxActive(bool bActive);
    void ShowSlot(bool bVisible);
    static FJ_SlotList_Slot* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    int getSlotId();
    bool getHighLightActive();
    bool getRedBoxActive();
};

#endif
