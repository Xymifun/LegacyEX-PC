#ifndef GAME_UI_UICONTROL_SLOTLIST_HPP
#define GAME_UI_UICONTROL_SLOTLIST_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_SlotList : public UIControl_Base
{
public:
    UIControl_SlotList();
    virtual ~UIControl_SlotList() {}
    void addSlot(unsigned int slot);
    void addSlots(unsigned int firstSlot, int count);
    void setSlotHighlighted(int slot, bool highlighted);
    void setHighlightSlot(int slot);
    void showSlotRedBox(int slot, bool show);
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    virtual void setFocus(bool focus);

private:
    int m_highlightSlot;
};

#endif
