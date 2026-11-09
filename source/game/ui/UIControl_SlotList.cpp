#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_SlotList::UIControl_SlotList() : m_highlightSlot(-1)
{
}

void UIControl_SlotList::addSlot(unsigned int slot)
{
    if (m_fuiRenderNode == NULL)
        return;
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node == NULL)
        return;
    FJ_SlotList* list = static_cast<FJ_SlotList*>(node);
    list->addSlot((int)slot, false, false);
}

void UIControl_SlotList::addSlots(unsigned int firstSlot, int count)
{
    for (unsigned int slot = firstSlot; slot < firstSlot + count; ++slot)
        addSlot(slot);
}

void UIControl_SlotList::setSlotHighlighted(int slot, bool highlighted)
{
    if (m_fuiRenderNode == NULL)
        return;
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node == NULL)
        return;
    FJ_SlotList* list = static_cast<FJ_SlotList*>(node);
    list->SetSlotHighlight(slot, highlighted);
}

void UIControl_SlotList::setHighlightSlot(int slot)
{
    if (slot == m_highlightSlot)
        return;
    if (m_highlightSlot != -1)
        setSlotHighlighted(m_highlightSlot, false);
    setSlotHighlighted(slot, true);
    m_highlightSlot = slot;
}

void UIControl_SlotList::showSlotRedBox(int slot, bool show)
{
    if (m_fuiRenderNode == NULL)
        return;
    FJ_FuiNode* node = m_fuiRenderNode->getFuiNode();
    if (node == NULL)
        return;
    FJ_SlotList* list = static_cast<FJ_SlotList*>(node);
    list->SetSlotRedBox(slot, show);
}

bool UIControl_SlotList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_SlotList;
    bool result = UIControl_Base::setupControl(scene, node, name);
    m_highlightSlot = 0;
    return result;
}

void UIControl_SlotList::ReInit()
{
    UIControl::ReInit();
    m_highlightSlot = -1;
}

void UIControl_SlotList::setFocus(bool focus)
{
    if (m_highlightSlot == -1)
        return;
    setSlotHighlighted(m_highlightSlot, focus);
}
