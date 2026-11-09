#include "game/ui/FJ_SlotList.hpp"

#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_SlotList_Slot.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_SlotList::FJ_SlotList(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    m_slotSize = 50;
    m_slotVisible = false;
    m_loaded = false;
}

FJ_FuiNode* FJ_SlotList::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_SlotList(pRenderNode);
}

unsigned int FJ_SlotList::getType()
{
    return 0x5e;
}

float FJ_SlotList::getWidth()
{
    return FJ_FuiNode::getWidth();
}

float FJ_SlotList::getHeight()
{
    return FJ_FuiNode::getHeight();
}

void FJ_SlotList::Init()
{
    if (m_loaded)
        return;
    setAlpha(0.5f);
    m_rows = Math::round(FJ_SlotList::getHeight() / (float)m_slotSize);
    m_columns = Math::round(FJ_SlotList::getWidth() / (float)m_slotSize);
    if (m_slotSize > 0)
        m_loaded = true;
    FJ_FuiNode* background = getChildByName("SlotListBackground")->asMovieClip();
    removeChild(background, true);
}

void FJ_SlotList::Clear()
{
    if (m_slots.empty())
        return;
    for (unsigned int i = 0; i < m_slots.size(); ++i)
    {
        if (m_slots[i]->getParent() != NULL)
        {
            FJ_SlotList_Slot*& slot = m_slots[i];
            FJ_FuiNode* parent = FJ_FuiNode::asMovieClip(slot)->getParent();
            FJ_FuiNode* grandParent = FJ_FuiNode::asMovieClip(slot)->getParent()->getParent();
            grandParent->removeChild(parent, true);
        }
    }
    m_slots.clear();
}

void FJ_SlotList::addSlot(int slot, bool highlighted, bool visible)
{
    if (!m_loaded)
        Init();
    if (!m_loaded)
        return;
    if (m_slots.size() >= (unsigned int)(m_rows * m_columns))
        return;

    FJ_FuiNode* container = FJ_Factory::createObject("", m_pRenderNode, true, NULL);
    getParent()->addChildAt(container, getParent()->getChildIndex(this));

    FJ_SlotList_Slot* slotNode = static_cast<FJ_SlotList_Slot*>(
        FJ_Factory::createObject("fourj.listClasses.FJ_SlotList_Slot", m_pRenderNode, true, NULL));
    slotNode->SetParams(m_slotSize, m_slotVisible);
    unsigned int row = m_slots.size() / (unsigned int)m_columns;
    unsigned int column = m_slots.size() % (unsigned int)m_columns;
    container->setX(getX() + (float)(int)(column * m_slotSize));
    container->setY(getY() + (float)(int)(row * m_slotSize));
    container->addChild(slotNode);
    slotNode->SetSlotID(slot);
    m_slots.push_back(slotNode);
    slotNode->SetHighlightActive(highlighted);
    slotNode->SetRedBoxActive(visible);
}

bool FJ_SlotList::SetSlotRedBox(int slot, bool show)
{
    if (!m_slots.empty() && slot >= 0 && (unsigned int)slot < m_slots.size() && m_slots[slot] != NULL)
    {
        m_slots[slot]->SetRedBoxActive(show);
        return true;
    }
    return false;
}

bool FJ_SlotList::SetSlotHighlight(int slot, bool highlighted)
{
    if (!m_slots.empty() && slot >= 0 && (unsigned int)slot < m_slots.size() && m_slots[slot] != NULL)
    {
        m_slots[slot]->SetHighlightActive(highlighted);
        return true;
    }
    return false;
}

void FJ_SlotList::setVisible(bool visible)
{
    if (m_slots.empty())
    {
        FJ_FuiNode::setVisible(visible);
        return;
    }
    for (unsigned int i = 0; i < m_slots.size(); ++i)
    {
        FJ_SlotList_Slot* slot = m_slots[i];
        if (slot->getParent() != NULL)
            m_slots[i]->getParent()->setVisible(visible);
    }
}
