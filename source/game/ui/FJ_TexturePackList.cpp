#include "game/ui/FJ_TexturePackList.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_ScrollArrowH.hpp"
#include "game/ui/FJ_TexturePackList_Slot.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

static std::string s_texturePackListStates[2] = {"Unselected", "Selected"};

FJ_TexturePackList::FJ_TexturePackList(fuiRenderNode* renderNode) : FJ_Base(renderNode), m_items()
{
    m_topSlot = 0;
    m_focusSlot = 0;
    m_visibleSlots = 0;
    m_touchSlot = 0;
    m_touchX = 0;
    m_selectedSlot = 0;
    m_disabledAlpha = 0.5f;
    m_selectorEnabled = true;
    m_touchMoved = false;
    m_slotArea = getChildByName(std::string("SlotArea"))->asMovieClip();
    m_slotArea->setVisible(false);
    m_leftArrow = static_cast<FJ_ScrollArrowH*>(FJ_Factory::createObject("fourj.FJ_ScrollArrowH", renderNode, true, 0));
    m_rightArrow = static_cast<FJ_ScrollArrowH*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrowH", renderNode, true, 0));
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    addEventListener(eFJEventType_UpdateLabel, this, _HandleUpdateLabel, false);
}

void FJ_TexturePackList::ChangeState(int state)
{
    if (currentLabel() == s_texturePackListStates[state])
        return;
    gotoAndPlay(s_texturePackListStates[state]);
    m_state = state;
}

void FJ_TexturePackList::focusInHandler(FJ_Event*)
{
    ChangeState(1);
}
void FJ_TexturePackList::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->focusInHandler(event);
}

void FJ_TexturePackList::HandleScrollListArrows(bool addArrows)
{
    if (addArrows)
    {
        unsigned int count = m_items.size();
        if (count >= static_cast<unsigned int>(m_visibleSlots) && m_leftArrow->getParent() == 0)
        {
            getParent()->addChild(m_leftArrow);
            getParent()->addChild(m_rightArrow);
            m_leftArrow->setX(getX() + static_cast<float>(m_slotSpacing / 2));
            m_leftArrow->setY(getY() + m_slotArea->getHeight() * 0.5f);
            m_rightArrow->setX(getX() + getWidth() - static_cast<float>(m_slotSpacing / 2));
            m_rightArrow->setY(m_leftArrow->getY());
            m_leftArrow->ChangeState(0);
            m_rightArrow->ChangeState(2);
        }
    }
    if (m_topSlot > 0)
        m_leftArrow->setVisible(true);
    else
        m_leftArrow->setVisible(false);
    if (static_cast<unsigned int>(m_topSlot) < m_items.size() - m_visibleSlots)
        m_rightArrow->setVisible(true);
    else
        m_rightArrow->setVisible(false);
}

void FJ_TexturePackList::ScrollList(bool down, bool page, bool fromTouch)
{
    if (!page && !fromTouch)
    {
        if (m_selectedSlot >= m_topSlot + m_visibleSlots)
        {
            m_topSlot = m_selectedSlot + 1 - m_visibleSlots;
            if (m_rightArrow != 0)
                m_rightArrow->ChangeState(3);
        }
        else if (m_selectedSlot < m_topSlot)
        {
            m_topSlot = m_selectedSlot;
            if (m_leftArrow != 0)
                m_leftArrow->ChangeState(1);
        }
    }
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        float x = getX() + m_items[i]->getWidth() * static_cast<float>(static_cast<int>(i) - m_topSlot)
                  + static_cast<float>(m_slotSpacing);
        m_items[i]->setX(x);
        if (static_cast<int>(i) < m_topSlot || static_cast<int>(i) >= m_topSlot + m_visibleSlots)
            m_items[i]->setVisible(false);
        else
            m_items[i]->setVisible(true);
        static_cast<FJ_TexturePackList_Slot*>(m_items[i])->SetSelected(i == static_cast<unsigned int>(m_selectedSlot));
    }
    if (!page)
        HandleScrollListArrows(false);
}

void FJ_TexturePackList::GotoSelectedSlot()
{
    if (m_selectedSlot > m_focusSlot)
    {
        m_selectedSlot = m_focusSlot;
        ScrollList(true, false, false);
    }
    else if (m_selectedSlot < m_focusSlot)
    {
        m_selectedSlot = m_focusSlot;
        ScrollList(false, false, false);
    }
}

void FJ_TexturePackList::focusOutHandler(FJ_Event*)
{
    ChangeState(0);
    if (m_selectedSlot != m_focusSlot)
        getRoot()->m_pCallbackScene->handleSelectionChanged(m_focusSlot);
    GotoSelectedSlot();
}
void FJ_TexturePackList::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->focusOutHandler(event);
}

void FJ_TexturePackList::NavigateList(bool down)
{
    if (!down)
    {
        m_selectedSlot--;
        if (m_selectedSlot < 0)
            m_selectedSlot = m_items.size() - 1;
    }
    else
    {
        m_selectedSlot++;
        if (static_cast<unsigned int>(m_selectedSlot) >= m_items.size())
            m_selectedSlot = 0;
    }
    getRoot()->m_pCallbackScene->handleSelectionChanged(m_selectedSlot);
    ScrollList(!down, false, false);
}

void FJ_TexturePackList::keyDownHandler(FJ_Event* event)
{
    if (!m_selectorEnabled)
        return;
    FJ_KeyboardEvent* keyEvent = static_cast<FJ_KeyboardEvent*>(event);
    int keyCode = keyEvent->m_iKeyCode;
    if (keyCode == 0)
    {
        m_focusSlot = m_selectedSlot;
        getRoot()->m_pCallbackScene->handlePress(getId(), m_focusSlot);
        keyCode = keyEvent->m_iKeyCode;
    }
    if (keyCode == 5)
        NavigateList(false);
    else if (keyCode == 6)
        NavigateList(true);
}
void FJ_TexturePackList::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->keyDownHandler(event);
}

void FJ_TexturePackList::UpdateLabel()
{
    SetLabel(m_label);
}
void FJ_TexturePackList::HandleUpdateLabel(FJ_Event*)
{
    UpdateLabel();
}
void FJ_TexturePackList::_HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_TexturePackList*>(node)->HandleUpdateLabel(event);
}

FJ_TexturePackList* FJ_TexturePackList::Create(fuiRenderNode* renderNode)
{
    return new FJ_TexturePackList(renderNode);
}

unsigned int FJ_TexturePackList::getType()
{
    return 0x8009;
}

void FJ_TexturePackList::Init(const std::wstring& name, int id)
{
    InitBase(name, id);
}

void FJ_TexturePackList::SelectSlot(int slot)
{
    if (static_cast<unsigned int>(slot) < m_items.size())
    {
        m_focusSlot = slot;
        GotoSelectedSlot();
    }
}

void FJ_TexturePackList::EnableSelector(bool enabled)
{
    unsigned int i = 0;
    if (enabled)
    {
        for (; i < m_items.size(); ++i)
            m_items[i]->setAlpha(1.0f);
        HandleScrollListArrows(false);
    }
    else
    {
        for (; i < m_items.size(); ++i)
        {
            if (i == static_cast<unsigned int>(m_focusSlot))
                m_items[i]->setAlpha(1.0f);
            else
                m_items[i]->setAlpha(m_disabledAlpha);
        }
        m_leftArrow->setVisible(false);
        m_rightArrow->setVisible(false);
    }
    m_selectorEnabled = enabled;
}

void FJ_TexturePackList::addPack(int id, const std::wstring& name)
{
    if (getParent() == 0)
        return;
    FJ_TexturePackList_Slot* slot = static_cast<FJ_TexturePackList_Slot*>(
        FJ_Factory::createObject("fourj.listClasses.FJ_TexturePackList_Slot", m_pRenderNode, true, 0));
    slot->SetSlotID(id);
    slot->SetTextureName(name);
    if (m_visibleSlots == 0)
    {
        m_visibleSlots = static_cast<int>(getWidth() / slot->getWidth() - 1.0f);
        m_slotSpacing = static_cast<int>((getWidth() - static_cast<float>(m_visibleSlots) * slot->getWidth()) * 0.5f);
    }
    slot->setY(getY() + Math::round((m_slotArea->getHeight() - slot->getHeight()) * 0.5f));
    m_items.push_back(slot);
    ScrollList(true, true, false);
    getParent()->addChild(slot);
    HandleScrollListArrows(true);
}

void FJ_TexturePackList::removeAllItems()
{
    unsigned int i;
    for (i = 0; i < m_items.size(); i++)
        getParent()->removeChild(m_items[i], true);
    m_items.clear();
    m_selectedSlot = 0;
    m_topSlot = 0;
    HandleScrollListArrows(false);
}

void FJ_TexturePackList::SetTouchFocus(int x, int y, bool focus)
{
    if (!m_selectorEnabled)
        return;
    x += static_cast<int>(getX());
    if (!focus)
    {
        float touchPoint = static_cast<float>(x);
        for (unsigned int i = 0; i < m_items.size(); ++i)
        {
            if (touchPoint > m_items[i]->getX() && touchPoint < m_items[i]->getX() + m_items[i]->getWidth())
            {
                m_touchSlot = i;
                m_touchX = x;
                m_touchMoved = false;
                return;
            }
        }
        m_touchX = x;
    }
    else if (!m_items.empty() && m_items.size() > static_cast<unsigned int>(m_visibleSlots))
    {
        if (m_touchX > x)
        {
            int amount = static_cast<int>(static_cast<float>(m_touchX - x) / m_items[0]->getHeight());
            if (amount > 0)
            {
                int topSlot = m_topSlot + amount;
                m_touchMoved = true;
                if (m_items.size() <= static_cast<unsigned int>(topSlot + m_visibleSlots))
                    m_topSlot = m_items.size() - m_visibleSlots;
                else
                    m_topSlot = topSlot;
                ScrollList(true, false, true);
                m_touchX = x;
                if (m_rightArrow != 0)
                    m_rightArrow->ChangeState(3);
            }
        }
        else if (m_touchX < x)
        {
            int amount = static_cast<int>(static_cast<float>(x - m_touchX) / m_items[0]->getHeight());
            if (amount > 0)
            {
                int topSlot = m_topSlot - amount;
                m_touchMoved = true;
                if (topSlot < 0)
                    topSlot = 0;
                m_topSlot = topSlot;
                ScrollList(false, false, true);
                m_touchX = x;
                if (m_leftArrow != 0)
                    m_leftArrow->ChangeState(1);
            }
        }
    }
}

unsigned int FJ_TexturePackList::CanTouchTrigger(int x, int y)
{
    if (!m_selectorEnabled)
        return 0;
    unsigned int triggered = 0;
    x += static_cast<int>(getX());
    float touchPoint = static_cast<float>(x);
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        if (touchPoint > m_items[i]->getX() && touchPoint < m_items[i]->getX() + m_items[i]->getWidth() && !m_touchMoved
            && m_touchSlot == static_cast<int>(i))
        {
            if (m_items[i]->isVisible())
            {
                m_selectedSlot = i;
                SelectSlot(m_touchSlot);
                triggered = 1;
                getRoot()->m_pCallbackScene->handleSelectionChanged(m_selectedSlot);
            }
            else if (static_cast<int>(i) < m_selectedSlot)
            {
                NavigateList(false);
            }
            else if (m_selectedSlot < static_cast<int>(i))
            {
                NavigateList(true);
            }
            break;
        }
    }
    for (unsigned int j = 0; j < m_items.size(); ++j)
        static_cast<FJ_TexturePackList_Slot*>(m_items[j])->SetSelected(j == static_cast<unsigned int>(m_selectedSlot));
    return triggered;
}

float FJ_TexturePackList::GetRealHeight()
{
    return m_slotArea->getHeight();
}
