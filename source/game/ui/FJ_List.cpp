#include "game/ui/FJ_List.hpp"

#include <cstdlib>
#include <cstring>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_ListButtonController.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/util/Math.hpp"

FJ_List::FJ_List(fuiRenderNode* renderNode) : FJ_Base(renderNode)
{
    m_background = 0;
    m_activeItem = 0;
    m_topItem = 0;
    m_verticalSpacing = 0;
    m_scrollChanged = false;
    m_maxItemsVisibleUp = 1;
    m_touchFocusY = 0;
    m_touchY = 0;
    m_checkboxTick = false;
    m_initialized = false;
    m_touchActive = false;
    m_blockTouchTrigger = false;
    m_keyDown = false;
    m_keyUp = false;
    m_highlightFromTouch = false;
#ifdef MINECRAFT_PC
    m_pcHovering = false;
#endif
    m_touchItem = 0;
    m_upArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, 0));
    m_downArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, 0));
    m_touchSize = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.3f);
    FJ_FuiNode* background = getChildByName(std::string("ButtonListBackground"))->asMovieClip();
    background->setVisible(false);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
}

FJ_List::~FJ_List()
{
    FJ_FuiNode* upArrow = m_upArrow;
    if (upArrow != 0)
    {
        FJ_FuiNode* parent = upArrow->getParent();
        if (parent != 0)
            parent->removeChild(upArrow, true);
    }
    FJ_FuiNode* downArrow = m_downArrow;
    if (downArrow != 0)
    {
        FJ_FuiNode* parent = downArrow->getParent();
        if (parent != 0)
            parent->removeChild(downArrow, true);
    }
}

uint32_t FJ_List::FindHighlightItem(int index, bool backward, bool wrap)
{
    if (!backward)
    {
        for (unsigned int i = static_cast<unsigned int>(index); i < m_items.size(); ++i)
        {
            if (!m_items[i]->isFJ_Label())
            {
                index = i;
                wrap = false;
                break;
            }
        }
    }
    else
    {
        for (int i = index; i >= 0; --i)
        {
            if (!m_items[i]->isFJ_Label())
            {
                index = i;
                wrap = false;
                break;
            }
        }
    }
    if (wrap)
        index = FindHighlightItem(index, !backward, false);
    return index;
}

void FJ_List::HighlightItem(int index, bool fromTouch)
{
    unsigned int item = static_cast<unsigned int>(index);
    if (item < m_items.size())
    {
        int activeItem = m_activeItem;
        if (m_items[item]->isFJ_Label())
            item = FindHighlightItem(item, static_cast<int>(item) < activeItem, true);
        m_highlightFromTouch = fromTouch;
        getStage()->setFocus(m_items[item]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
    }
}

void FJ_List::HandlePageUpDownNavigation(bool up)
{
    int item = m_topItem;
    if (up)
    {
        if (m_activeItem <= item)
        {
            item = m_activeItem - m_maxItemsVisibleUp;
            item = item < 0 ? 0 : item;
        }
    }
    else
    {
        item += m_maxItemsVisibleUp - 1;
        if (item <= m_activeItem)
        {
            item = m_activeItem + m_maxItemsVisibleUp;
            if (item >= static_cast<int>(m_items.size()))
                item = static_cast<int>(m_items.size()) - 1;
        }
    }
    HighlightItem(item, false);
}

void FJ_List::LoseListFocus()
{
    FJ_FuiNode* focus = getStage()->getFocus();
    if (!focus->isFJ_Base())
    {
        if (focus->isFJ_List() && getStage()->getFocus() != this && m_background != 0)
            m_background->gotoAndPlay(std::string("FadeOut"));
    }
    else
    {
        FJ_Base* focusedBase = focus->asFJ_Base();
        if (focusedBase->getObjListParent() != 0)
        {
            if (focusedBase->getObjListParent() != this)
            {
                if (m_background != 0)
                    m_background->gotoAndPlay(std::string("FadeOut"));
                return;
            }
        }
        if (focusedBase->getObjListParent() == 0 && m_background != 0)
            m_background->gotoAndPlay(std::string("FadeOut"));
    }
}

int FJ_List::getINumButtons()
{
    return static_cast<int>(m_items.size());
}

void FJ_List::SetListFocus()
{
    if (getStage()->getFocus() != this)
        return;

    int index = m_activeItem;
    if (m_initialized)
    {
        if (m_keyDown)
            index = static_cast<int>(m_items.size()) - 1;
        else if (m_keyUp)
            index = 0;
    }

    if (!m_items.empty())
    {
        getStage()->setFocus(m_items[static_cast<unsigned int>(index)]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
    }
    if (m_background != 0)
        m_background->gotoAndPlay(std::string("FadeIn"));
}

void FJ_List::focusInHandler(FJ_Event*)
{
    SetListFocus();
}
void FJ_List::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List*>(node)->focusInHandler(event);
}

void FJ_List::focusOutHandler(FJ_Event*)
{
    if (!m_items.empty())
        return;
    if (m_background != 0)
        m_background->gotoAndPlay(std::string("FadeOut"));
}

void FJ_List::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List*>(node)->focusOutHandler(event);
}

void FJ_List::Init(int id)
{
    InitBase(std::wstring(L""), id);
    if (getParent()->getChildByName(m_action) != 0)
        m_background = getParent()->getChildByName(m_action)->asMovieClip();
    m_touchSize = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.3f);
}

void FJ_List::UpdateScrollListArrows()
{
    if (m_upArrow != 0 && m_downArrow != 0)
    {
        m_upArrow->setX(static_cast<float>(Math::round((getX() + getWidth()) - m_upArrow->getWidth() * 1.5f)));
        m_upArrow->setY(static_cast<float>(Math::round((getY() + getHeight()) - m_upArrow->getHeight() * 0.5f)));
        m_downArrow->setX(static_cast<float>(Math::round((getX() + getWidth()) - m_upArrow->getWidth() * 0.5f)));
        m_downArrow->setY(m_upArrow->getY());
    }
}

void FJ_List::setX(float x)
{
    FJ_FuiNode::setX(x);
    UpdateScrollListArrows();
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        m_items[i]->setX(getX());
        for (unsigned int j = 0; j < static_cast<FJ_Base*>(m_items[i])->m_objects.size(); ++j)
            static_cast<FJ_Base*>(m_items[i])->m_objects[j]->setX(m_items[i]->getX());
    }
}

void FJ_List::setY(float y)
{
    float old = getY();
    FJ_FuiNode::setY(y);
    UpdateScrollListArrows();
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        m_items[i]->incY(y - old);
        for (std::vector<FJ_FuiNode*>::iterator child = static_cast<FJ_Base*>(m_items[i])->m_objects.begin();
             child != static_cast<FJ_Base*>(m_items[i])->m_objects.end(); ++child)
            (*child)->setY(m_items[i]->getY());
    }
}

void FJ_List::ListKeyDownHandler(FJ_Event* event)
{
    m_keyDown = false;
    m_keyUp = false;
    if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 3)
        m_keyDown = true;
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 4)
        m_keyUp = true;
}
void FJ_List::_ListKeyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List*>(node)->ListKeyDownHandler(event);
}

float FJ_List::GetListItemHeight(int index)
{
    if (static_cast<unsigned int>(index) >= m_items.size())
        return 0.0f;
    if (m_items[index]->isFJ_ListButtonController())
        return m_items[index]->asFJ_ListButtonController()->getOriginalHeight();
    return m_items[index]->getHeight();
}

void FJ_List::GetMaxItemsVisibleUp()
{
    int height = 0;
    int count = 0;
    for (int i = m_activeItem; i >= 0; --i)
    {
        height += Math::round(static_cast<float>(GetListItemHeight(i))) + (i > 0 ? m_verticalSpacing : 0);
        if (height >= m_touchSize)
            break;
        ++count;
    }
    m_maxItemsVisibleUp = count;
}

std::wstring FJ_List::GetItemLabel(int index)
{
    if (static_cast<unsigned int>(index) >= m_items.size())
        return std::wstring(L"");
    return m_items[index]->asFJ_Base()->GetLabel();
}

void FJ_List::HandleScrollListArrows(bool addArrows)
{
    if (addArrows && m_items.size() >= static_cast<unsigned int>(m_maxItemsVisibleUp))
    {
        FJ_FuiNode* upArrow = m_upArrow;
        if (upArrow->getParent() == 0)
        {
            getParent()->addChild(upArrow);
            getParent()->addChild(m_downArrow);
            m_upArrow->setX(static_cast<float>(Math::round((getX() + getWidth()) - m_upArrow->getWidth() * 1.5f)));
            m_upArrow->setY(static_cast<float>(Math::round((getY() + getHeight()) - m_upArrow->getHeight() * 0.5f)));
            m_downArrow->setX(static_cast<float>(Math::round((getX() + getWidth()) - m_upArrow->getWidth() * 0.5f)));
            m_downArrow->setY(m_upArrow->getY());
            m_upArrow->ChangeState(0);
            m_downArrow->ChangeState(3);
        }
    }
    if (m_topItem > 0)
        m_upArrow->setVisible(true);
    else
        m_upArrow->setVisible(false);
    if (m_topItem < static_cast<int>(m_items.size()) - m_maxItemsVisibleUp)
        m_downArrow->setVisible(true);
    else
        m_downArrow->setVisible(false);
}

void FJ_List::ScrollList(bool down, bool page, bool fromTouch)
{
#ifdef MINECRAFT_PC
    if (m_pcHovering)
        return;
#endif
    if (!page)
    {
        if (m_highlightFromTouch)
        {
            int overflow = m_activeItem + m_maxItemsVisibleUp - static_cast<int>(m_items.size());
            down = false;
            m_highlightFromTouch = false;
            if (overflow < 1)
                m_topItem = m_activeItem;
            else
                m_topItem = m_activeItem - overflow;
        }
        else if (!down)
        {
            if (m_topItem + m_maxItemsVisibleUp <= m_activeItem)
            {
                GetMaxItemsVisibleUp();
                m_topItem = m_activeItem + 1 - m_maxItemsVisibleUp;
                if (m_downArrow != 0)
                    m_downArrow->ChangeState(4);
            }
        }
        else if (m_activeItem < m_topItem)
        {
            m_topItem = m_activeItem;
            if (m_upArrow != 0)
                m_upArrow->ChangeState(1);
        }
    }
    if (down && m_activeItem > 0 && m_items[m_activeItem - 1]->isFJ_Label() && GetItemLabel(m_activeItem - 1) != L" "
        && !m_items[m_activeItem - 1]->isVisible())
        --m_topItem;
    if (!down && static_cast<unsigned int>(m_activeItem + 2) < m_items.size() && m_items[m_activeItem + 1]->isFJ_Label()
        && GetItemLabel(m_activeItem + 1) != L" " && !m_items[m_activeItem + 1]->isVisible())
        ++m_topItem;

    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        float beforeTop = 0.0f;
        float beforeItem = 0.0f;
        float visibleHeight = 0.0f;
        bool reachedLimit = false;
        for (unsigned int j = 0; j < m_items.size(); ++j)
        {
            if (static_cast<int>(j) < m_topItem)
                beforeTop += GetListItemHeight(j) + static_cast<float>(m_verticalSpacing);
            if (static_cast<int>(j) < static_cast<int>(i))
                beforeItem += GetListItemHeight(j) + static_cast<float>(m_verticalSpacing);
            if (static_cast<int>(j) >= m_topItem && !reachedLimit)
            {
                visibleHeight += GetListItemHeight(j);
                m_maxItemsVisibleUp = static_cast<int>(j) - m_topItem;
                if (static_cast<float>(m_touchSize) < visibleHeight)
                {
                    reachedLimit = true;
                    continue;
                }
                visibleHeight += static_cast<float>(m_verticalSpacing);
            }
        }
        FJ_FuiNode* item = m_items[i];
        if (!reachedLimit)
            ++m_maxItemsVisibleUp;
        item->setY(static_cast<float>(Math::round(getY() + beforeItem - item->getRect(item).yMin - beforeTop)));
        if (m_items[i]->isFJ_Base())
        {
            for (unsigned int j = 0; j < m_items[i]->asFJ_Base()->m_objects.size(); ++j)
                m_items[i]->asFJ_Base()->m_objects[j]->setY(m_items[i]->getY());
        }
        if (static_cast<int>(i) < m_topItem || m_topItem + m_maxItemsVisibleUp <= static_cast<int>(i))
            m_items[i]->setVisible(false);
        else
            m_items[i]->setVisible(true);
        for (unsigned int j = 0; j < m_items[i]->asFJ_Base()->m_objects.size(); ++j)
            m_items[i]->asFJ_Base()->m_objects[j]->setVisible(m_items[i]->isVisible());
    }
    if (!page)
        HandleScrollListArrows(false);
}

void FJ_List::ItemFocusInHandler(FJ_Event* event)
{
    int newItem = event->m_pTarget->asFJ_Base()->getListId();
    int oldItem = m_activeItem;
    m_activeItem = event->m_pTarget->asFJ_Base()->getListId();
    ScrollList(newItem < oldItem, false, false);
}
void FJ_List::_ItemFocusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List*>(node)->ItemFocusInHandler(event);
}

int FJ_List::GetListHeight()
{
    int result = static_cast<int>(getHeight());
    if (!m_upArrow->isVisible() && !m_downArrow->isVisible())
    {
        result = 0;
        for (unsigned int i = 0; i < m_items.size(); ++i)
            result += Math::round(GetListItemHeight(static_cast<int>(i)) + static_cast<float>(m_verticalSpacing));
    }
    return result;
}

void FJ_List::UpdateTouchSize()
{
    FJ_FuiNode* background = getChildByName(std::string("ButtonListBackground"))->asMovieClip();
    if (background != 0)
    {
        int height = GetListHeight() - m_verticalSpacing;
        background->setHeight(static_cast<float>(Math::round(static_cast<float>(height) / getScaleY())));
    }
}
int FJ_List::GetListWidth()
{
    int result = static_cast<int>(getWidth());
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        int itemWidth;
        if (m_items[i]->isFJ_Slider())
        {
            itemWidth = Math::round(m_items[i]->asFJ_Base()->GetTextField()->m_textWidth);
            if (itemWidth < m_items[i]->asFJ_Base()->getOldWidth())
                itemWidth = static_cast<int>(m_items[i]->asFJ_Base()->GetTextField()->m_textWidth * 1.1f);
        }
        else
        {
            itemWidth = Math::round(m_items[i]->asFJ_Base()->getWidth());
            int newWidth = m_items[i]->asFJ_Base()->getNewWidth();
            if (itemWidth < newWidth)
                itemWidth = newWidth;
        }
        if (result < itemWidth)
            result = itemWidth;
    }
    return result;
}
void FJ_List::SetNewObjectWidth(int width)
{
    if (getWidth() < width)
        setWidth(static_cast<float>(width));
}

void FJ_List::addNewListItem(FJ_Base* item, const std::wstring& label, int id)
{
    const int index = static_cast<int>(m_items.size());
    item->setId(id);
    item->setListId(index);
    item->setObjListParent(this);
    if (!item->isFJ_CheckBox())
        item->setWidth(getWidth());
    item->setPreScaleWidth(static_cast<int>(item->getWidth()));
    if (!item->isFJ_CheckBox())
    {
        item->setX(getX() + static_cast<float>(Math::round((getWidth() - item->getWidth()) * 0.5f)));
    }
    else
        item->setX(getX());
    m_items.push_back(item);
    item->m_navLeftName = m_navLeftName;
    item->m_navRightName = m_navRightName;
    item->m_navDownName = m_navDownName;
    if (index > 0)
    {
        item->m_navUpName = m_items[index - 1]->asFJ_Base()->getName();
        m_items[index - 1]->asFJ_Base()->m_navDownName = item->getName();
    }
    else
        item->m_navUpName = m_navUpName;
    (void)label;
    item->addEventListener(eFJEventType_FocusIn, this, _ItemFocusInHandler, false);
    getParent()->addChild(item);
    if (m_initialized != 0)
    {
        if (m_navUpName.empty() && m_navDownName.empty())
        {
            item->m_navDownName = m_items[0]->getName();
            m_items[0]->asFJ_Base()->m_navUpName = item->getName();
        }
        else
            getParent()->addEventListener(eFJEventType_KeyDown, this, _ListKeyDownHandler, false);
    }
    if (index == m_activeItem)
        SetListFocus();
    ScrollList(false, true, false);
    if (m_activeItem == 0 && m_items[0]->isFJ_Label())
    {
        for (unsigned int i = 0; i < m_items.size(); ++i)
        {
            if (!m_items[i]->isFJ_Label())
            {
                m_activeItem = static_cast<int>(i);
                SetListFocus();
                break;
            }
        }
    }
    HandleScrollListArrows(true);
}

void FJ_List::removeAllItems()
{
    bool hadFocus = false;
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        if (getStage()->getFocus() == m_items[i])
        {
            hadFocus = true;
            getStage()->removeFocus(m_items[i]);
        }
        if (m_items[i]->isFJ_Base())
        {
            for (unsigned int j = 0; j < m_items[i]->asFJ_Base()->m_objects.size(); ++j)
                getParent()->removeChild(m_items[i]->asFJ_Base()->m_objects[j], true);
        }
        getParent()->removeChild(m_items[i], true);
    }
    m_activeItem = 0;
    m_items.clear();
    m_topItem = 0;
    HandleScrollListArrows(false);
    if (hadFocus)
        getStage()->setFocus(this);
}

void FJ_List::SetButtonLabel(int index, const std::wstring& label)
{
    if (static_cast<unsigned int>(index) < m_items.size())
        m_items[index]->asFJ_Base()->SetLabel(label);
}

void FJ_List::SetItemAlpha(int index, float alpha)
{
    if (static_cast<unsigned int>(index) < m_items.size())
        m_items[index]->asFJ_Base()->setAlpha(alpha);
}

void FJ_List::SetItemLabel(int index, const std::wstring& label)
{
    if (static_cast<unsigned int>(index) < m_items.size())
        m_items[index]->asFJ_Base()->SetLabel(label);
}

void FJ_List::EnableItem(int index, bool enabled)
{
    if (static_cast<unsigned int>(index) < m_items.size())
    {
        if (m_items[index]->isFJ_Button())
            m_items[index]->asFJ_Button()->EnableButton(enabled);
        else if (m_items[index]->isFJ_CheckBox())
            m_items[index]->asFJ_CheckBox()->EnableCheckBox(enabled);
        else if (m_items[index]->isFJ_Slider())
            m_items[index]->asFJ_Slider()->EnableSlider(enabled);
    }
}

void FJ_List::SetTouchFocus(int x, int y, bool focus)
{
    if (!focus)
    {
        for (unsigned int i = 0; i < m_items.size(); ++i)
        {
            if (m_items[i]->getY() < static_cast<float>(y)
                && static_cast<float>(y)
                       < m_items[i]->getY() + static_cast<float>(GetListItemHeight(static_cast<int>(i))))
            {
                m_scrollChanged = false;
                m_touchActive = false;
                m_touchItem = static_cast<int>(i);
                HighlightItem(static_cast<int>(i), false);
                m_touchFocusY = y;
                m_touchY = x;
                return;
            }
        }
        m_touchFocusY = y;
        m_touchY = x;
        return;
    }

    if (!m_items.empty() && static_cast<unsigned int>(m_maxItemsVisibleUp) < m_items.size() && !m_touchActive)
    {
        if (m_touchFocusY < y)
        {
            int rows = static_cast<int>(
                static_cast<float>(y - m_touchFocusY)
                / (static_cast<float>(GetListItemHeight(0)) + static_cast<float>(m_verticalSpacing)));
            if (rows > 0)
            {
                int newTop = m_topItem - rows;
                m_scrollChanged = true;
                if (newTop < 0)
                    newTop = 0;
                m_topItem = newTop;
                ScrollList(true, false, true);
                m_touchFocusY = y;
                if (m_upArrow != 0)
                    m_upArrow->ChangeState(1);
                int lastVisible = m_topItem + m_maxItemsVisibleUp;
                if (lastVisible - 1 < m_activeItem)
                    HighlightItem(lastVisible - 1, false);
            }
        }
        else if (y < m_touchFocusY)
        {
            int rows = static_cast<int>(
                static_cast<float>(m_touchFocusY - y)
                / (static_cast<float>(GetListItemHeight(0)) + static_cast<float>(m_verticalSpacing)));
            if (rows > 0)
            {
                int newTop = m_topItem + rows;
                m_scrollChanged = true;
                if (m_items.size() <= static_cast<unsigned int>(newTop + m_maxItemsVisibleUp))
                    newTop = static_cast<int>(m_items.size()) - m_maxItemsVisibleUp;
                m_topItem = newTop;
                ScrollList(false, false, true);
                m_touchFocusY = y;
                if (m_downArrow != 0)
                    m_downArrow->ChangeState(4);
                if (m_activeItem < m_topItem)
                    HighlightItem(m_topItem, false);
            }
        }
    }

    if (!m_scrollChanged && !m_items.empty() && m_items[m_activeItem]->isFJ_Slider())
    {
        FJ_Slider* slider = m_items[m_activeItem]->asFJ_Slider();
#ifdef MINECRAFT_PC
        const int deadZone = 0;
#else
        const int deadZone = 50;
#endif
        if (slider->getEnabled() && (std::abs(x - m_touchY) > deadZone || m_touchActive))
        {
            m_items[m_activeItem]->asFJ_Slider()->SetRelativeSliderPos(static_cast<float>(x) / getWidth());
            m_touchActive = true;
        }
    }
}

#ifdef MINECRAFT_PC
void FJ_List::PcPressSlider(int x, int y)
{
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i]->getY() < static_cast<float>(y)
            && static_cast<float>(y) < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i)))
        {
            if (m_items[i]->isFJ_Slider() && m_items[i]->asFJ_Slider()->getEnabled())
            {
                m_items[i]->asFJ_Slider()->SetRelativeSliderPos(static_cast<float>(x) / getWidth());
                m_touchActive = true;
            }
            return;
        }
    }
}

void FJ_List::PcHoverItem(int y)
{
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i]->getY() < static_cast<float>(y)
            && static_cast<float>(y) < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i)))
        {
            if (m_items[i]->isVisible() && !m_items[i]->isFJ_Label())
            {
                m_pcHovering = true;
                HighlightItem(static_cast<int>(i), false);
                m_pcHovering = false;
            }
            return;
        }
    }
}
#endif

uint32_t FJ_List::CanTouchFocus(int, int y)
{
    for (unsigned int i = 0; i < m_items.size(); ++i)
    {
        if (m_items[i]->getY() < static_cast<float>(y)
            && static_cast<float>(y) < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i))
            && m_items[i]->isFJ_Label())
            return 0;
    }
    return 1;
}
void FJ_List::BlockTouchTrigger(bool block)
{
    m_blockTouchTrigger = block;
}
uint32_t FJ_List::CanTouchTrigger(int, int y)
{
    if (!m_blockTouchTrigger)
    {
        for (unsigned int i = 0; i < m_items.size(); ++i)
        {
            if (m_items[i]->getY() < static_cast<float>(y)
                && static_cast<float>(y) < m_items[i]->getY() + GetListItemHeight(static_cast<int>(i)))
            {
                if (m_items[i]->isFJ_CheckBox())
                    FJ_CheckBox::s_keyDown = false;
                if (!m_scrollChanged && m_touchItem == static_cast<int>(i))
                    return 1;
            }
        }
    }
    return 0;
}
void FJ_List::FixNavigation(FJ_FuiNode* node)
{
    FJ_Base* nodeBase = node->asFJ_Base();
    if (!nodeBase->m_navDownName.empty())
    {
        FJ_FuiNode* next = getParent()->getChildByName(nodeBase->m_navDownName);
        if (next != 0 && next->asMovieClip() != 0)
            next->asFJ_Base()->m_navUpName = nodeBase->m_navUpName;
    }
    if (!nodeBase->m_navUpName.empty())
    {
        FJ_FuiNode* prev = getParent()->getChildByName(nodeBase->m_navUpName);
        if (prev != 0 && prev->asMovieClip() != 0)
            prev->asFJ_Base()->m_navDownName = nodeBase->m_navDownName;
    }
}
void FJ_List::RemoveItem(int index)
{
    if (static_cast<unsigned int>(index) >= m_items.size())
        return;
    FixNavigation(m_items[index]);
    if (m_items[index]->isFJ_Base())
    {
        for (unsigned int i = 0; i < m_items[index]->asFJ_Base()->m_objects.size(); ++i)
            getParent()->removeChild(m_items[index]->asFJ_Base()->m_objects[i], true);
    }
    getParent()->removeChild(m_items[index], true);
    m_items.erase(m_items.begin() + index);
    for (unsigned int i = 0; i < m_items.size(); ++i)
        m_items[i]->asFJ_Base()->setListId(static_cast<int>(i));

    if (!m_items.empty())
    {
        if (m_topItem > 0
            && (m_activeItem > index || static_cast<unsigned int>(m_topItem + m_maxItemsVisibleUp) > m_items.size()))
            --m_topItem;
        ScrollList(true, true, false);
    }
    if (index <= m_activeItem)
    {
        if (m_activeItem > 0)
            --m_activeItem;
        if (static_cast<unsigned int>(m_activeItem) < m_items.size())
            getStage()->setFocus(m_items[m_activeItem]);
    }
    HandleScrollListArrows(true);
}
FJ_FuiNode* FJ_List::ReturnItem(int index)
{
    if (static_cast<unsigned int>(index) < m_items.size())
        return m_items[index]->asFJ_Base();
    return 0;
}
int FJ_List::GetActiveItem()
{
    FJ_FuiNode*& item = m_items[m_activeItem];
    if (getStage()->getFocus() == item)
        return m_activeItem;
    return -1;
}
uint32_t FJ_List::GetTopItem()
{
    return static_cast<uint32_t>(m_topItem);
}

void FJ_List::setAction(unsigned char action, char* value)
{
    switch (action)
    {
    case 10:
        m_checkboxTick = strcmp(value, "true") == 0;
        break;
    case 11:
        m_verticalSpacing = std::atoi(value);
        break;
    case 13:
        m_initialized = strcmp(value, "true") == 0;
        break;
    case 12:
        m_action.assign(value);
        break;
    }
}
