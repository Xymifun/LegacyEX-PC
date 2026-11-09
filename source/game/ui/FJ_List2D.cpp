#include "game/ui/FJ_List2D.hpp"

#include <cstdlib>
#include <cstring>
#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ListButtonAchievement.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_List2D::FJ_List2D(fuiRenderNode* renderNode) : FJ_Base(renderNode), m_items()
{
    m_activeItem = 0;
    m_firstVisibleRow = 0;
    m_activeRow = 0;
    m_activeColumn = 0;
    m_outline = false;
    m_itemSpacing = 0;
    m_visibleRows = 1;
    m_itemsPerRow = 1;
    m_touchY = 0;
    m_touchFocus = false;
    m_touchX = 0;
    m_touchRow = 0;
    m_scrollDelegate = 0;
    m_upArrow = static_cast<FJ_ScrollArrow*>(FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
    m_downArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", renderNode, true, NULL));
    m_listHeight = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.3f);
    FJ_FuiNode* background = getChildByName(std::string("ButtonListBackground"))->asMovieClip();
    background->setVisible(false);
    addEventListener(eFJEventType_FocusIn, this, _focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, _focusOutHandler, false);
}

void FJ_List2D::HighlightItem(int index)
{
    int row = index / m_itemsPerRow;
    int column = index - row * m_itemsPerRow;
    if (m_items[row].size() - 1 < static_cast<unsigned int>(column))
        column = static_cast<int>(m_items[row].size()) - 1;
    if (static_cast<unsigned int>(row) < m_items.size() && static_cast<unsigned int>(column) < m_items[row].size())
    {
        m_activeRow = row;
        m_activeColumn = column;
        getStage()->setFocus(m_items[row][column]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
    }
}

void FJ_List2D::HandlePageUpDownNavigation(bool up)
{
    if (up)
    {
        if (m_activeRow > m_firstVisibleRow)
            HighlightItem(m_firstVisibleRow * m_itemsPerRow + m_activeColumn);
        else
        {
            int targetIndex = m_activeColumn;
            int targetRow = m_activeRow - m_visibleRows;
            if (targetRow > 0)
                targetIndex = targetRow * m_itemsPerRow + targetIndex;
            HighlightItem(targetIndex);
        }
    }
    else
    {
        int targetRow = m_firstVisibleRow + m_visibleRows - 1;
        if (m_activeRow < targetRow)
            HighlightItem(targetRow * m_itemsPerRow + m_activeColumn);
        else
        {
            unsigned int lastRow = m_activeRow + m_visibleRows;
            if (lastRow >= m_items.size())
                lastRow = m_items.size() - 1;
            HighlightItem(lastRow * m_itemsPerRow + m_activeColumn);
        }
    }
}

FJ_FuiNode* FJ_List2D::getActiveItem()
{
    if (m_items.size() != 0)
        return m_items[m_activeRow][m_activeColumn];
    return 0;
}

void FJ_List2D::SetListFocus()
{
    if (getStage()->getFocus() == this)
    {
        int row = m_activeItem / m_itemsPerRow;
        int column = m_activeItem - m_itemsPerRow * row;
        if (!m_items.empty())
        {
            m_activeColumn = column;
            m_activeRow = row;
            getStage()->setFocus(m_items[row][column]);
            dispatchEvent(new FJ_Event(eFJEventType_FocusChangeInit, true, false));
        }
    }
}

void FJ_List2D::focusInHandler(FJ_Event*)
{
    SetListFocus();
}

void FJ_List2D::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List2D*>(node)->focusInHandler(event);
}

void FJ_List2D::_focusOutHandler(FJ_FuiNode*, FJ_Event*)
{
}

void FJ_List2D::Init(int id)
{
    InitBase(std::wstring(L""), id);
    m_listHeight = static_cast<int>(getHeight() - m_upArrow->getHeight() / 1.3f);
}

void FJ_List2D::UpdateScrollListArrows()
{
    if (m_upArrow == 0 || m_downArrow == 0)
        return;
    m_upArrow->setX(static_cast<float>(Math::round(getX() + getWidth() - m_upArrow->getWidth() * 1.5f)));
    m_upArrow->setY(static_cast<float>(Math::round(getY() + getHeight() - m_upArrow->getHeight() * 0.5f)));
    m_downArrow->setX(static_cast<float>(Math::round(getX() + getWidth() - m_upArrow->getWidth() * 0.5f)));
    m_downArrow->setY(m_upArrow->getY());
}

void FJ_List2D::setX(float x)
{
    FJ_FuiNode::setX(x);
    UpdateScrollListArrows();
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            m_items[row][column]->setX(getX());
            unsigned int object;
            for (object = 0; object < static_cast<FJ_Base*>(m_items[row][column])->m_objects.size(); ++object)
                static_cast<FJ_Base*>(m_items[row][column])->m_objects[object]->setX(m_items[row][column]->getX());
        }
    }
}

void FJ_List2D::setY(float y)
{
    FJ_FuiNode::setY(y);
    UpdateScrollListArrows();
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            m_items[row][column]->setY(getY());
            unsigned int object;
            for (object = 0; object < static_cast<FJ_Base*>(m_items[row][column])->m_objects.size(); ++object)
                static_cast<FJ_Base*>(m_items[row][column])->m_objects[object]->setY(m_items[row][column]->getY());
        }
    }
}

void FJ_List2D::SetScrollDelegate(void (*delegate)(FJ_FuiNode*), FJ_FuiNode* node)
{
    m_scrollDelegate = delegate;
    m_scrollDelegateNode = node;
}

void FJ_List2D::HandleScrollListArrows(bool addToStage)
{
    if (addToStage && m_items.size() >= static_cast<unsigned int>(m_visibleRows))
    {
        FJ_ScrollArrow* upArrow = m_upArrow;
        if (upArrow->getParent() == 0)
        {
            getParent()->addChild(upArrow);
            getParent()->addChild(m_downArrow);
            m_upArrow->setX(static_cast<float>(Math::round(getX() + getWidth() - m_upArrow->getWidth() * 1.5f)));
            m_upArrow->setY(static_cast<float>(Math::round(getY() + getHeight() - m_upArrow->getHeight() * 0.5f)));
            m_downArrow->setX(static_cast<float>(Math::round(getX() + getWidth() - m_upArrow->getWidth() * 0.5f)));
            m_downArrow->setY(m_upArrow->getY());
            m_upArrow->ChangeState(0);
            m_downArrow->ChangeState(3);
        }
    }
    if (m_firstVisibleRow > 0)
        m_upArrow->setVisible(true);
    else
        m_upArrow->setVisible(false);
    if (static_cast<unsigned int>(m_firstVisibleRow) < m_items.size() - m_visibleRows)
        m_downArrow->setVisible(true);
    else
        m_downArrow->setVisible(false);
}

void FJ_List2D::ScrollList(bool up, bool initial, bool immediate)
{
    if (!initial)
    {
        if (!up)
        {
            if (m_activeRow >= m_firstVisibleRow + m_visibleRows)
            {
                m_firstVisibleRow = m_activeRow + 1 - m_visibleRows;
                if (m_downArrow != 0)
                    m_downArrow->ChangeState(4);
            }
        }
        else if (m_activeRow < m_firstVisibleRow)
        {
            m_firstVisibleRow = m_activeRow;
            if (m_upArrow != 0)
                m_upArrow->ChangeState(1);
        }
    }

    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        bool reachedLimit = false;
        float heightBeforeFirstVisible = 0.0f;
        float heightBeforeRow = 0.0f;
        float visibleHeight = 0.0f;
        unsigned int r;
        for (r = 0; r < m_items.size(); ++r)
        {
            if (static_cast<int>(r) < m_firstVisibleRow)
                heightBeforeFirstVisible += m_items[r][0]->getHeight() + static_cast<float>(m_itemSpacing);
            if (static_cast<int>(r) < static_cast<int>(row))
                heightBeforeRow += m_items[r][0]->getHeight() + static_cast<float>(m_itemSpacing);
            if (static_cast<int>(r) >= m_firstVisibleRow && !reachedLimit)
            {
                visibleHeight += m_items[r][0]->getHeight();
                m_visibleRows = static_cast<int>(r) - m_firstVisibleRow;
                if (visibleHeight > static_cast<float>(m_listHeight))
                    reachedLimit = true;
                else
                    visibleHeight += static_cast<float>(m_itemSpacing);
            }
        }
        if (!reachedLimit)
            m_visibleRows += 1;

        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            FJ_FuiNode* itemX = m_items[row][column];
            fuiRect rect = itemX->getRect(itemX);
            itemX->setX(static_cast<float>(
                Math::round(getX() + static_cast<float>(static_cast<int>(column)) * m_items[0][0]->getWidth()
                            + static_cast<float>(static_cast<int>(column) * m_itemSpacing) - rect.xMin)));
            FJ_FuiNode* itemY = m_items[row][column];
            rect = itemY->getRect(itemY);
            itemY->setY(
                static_cast<float>(Math::round(getY() + heightBeforeRow - rect.yMin - heightBeforeFirstVisible)));
            if (m_items[row][column]->isFJ_Base())
            {
                unsigned int object;
                for (object = 0; object < m_items[row][column]->asFJ_Base()->m_objects.size(); ++object)
                    m_items[row][column]->asFJ_Base()->m_objects[object]->setY(m_items[row][column]->getY());
            }
        }

        if (static_cast<int>(row) < m_firstVisibleRow || m_firstVisibleRow + m_visibleRows <= static_cast<int>(row))
        {
            if (!immediate && row == static_cast<unsigned int>(m_activeRow))
            {
                ScrollList(up, initial, false);
                break;
            }
            for (column = 0; column < m_items[row].size(); ++column)
                m_items[row][column]->setVisible(false);
        }
        else
        {
            for (column = 0; column < m_items[row].size(); ++column)
                m_items[row][column]->setVisible(true);
        }

        for (column = 0; column < m_items[row].size(); ++column)
        {
            unsigned int object;
            for (object = 0; object < m_items[row][column]->asFJ_Base()->m_objects.size(); ++object)
                m_items[row][column]->asFJ_Base()->m_objects[object]->setVisible(m_items[row][column]->isVisible());
        }
    }

    if (!initial)
        HandleScrollListArrows(false);
    if (m_scrollDelegate != 0)
        m_scrollDelegate(m_scrollDelegateNode);
}

void FJ_List2D::ItemFocusInHandler(FJ_Event* event)
{
    int listId = static_cast<int>(event->m_pTarget->asFJ_Base()->getListId());
    int previousItem = m_activeItem;
    bool up = listId < previousItem;
    int activeItem = static_cast<int>(event->m_pTarget->asFJ_Base()->getListId());
    int row = activeItem / m_itemsPerRow;
    m_activeRow = row;
    m_activeItem = activeItem;
    m_activeColumn = activeItem - m_itemsPerRow * row;
    ScrollList(up, false, false);
}

void FJ_List2D::_ItemFocusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_List2D*>(node)->ItemFocusInHandler(event);
}

void FJ_List2D::SetNewObjectWidth(int width)
{
    float newWidth = static_cast<float>(width);
    float currentWidth = getWidth();
    if (newWidth > currentWidth)
        setWidth(newWidth);
}

void FJ_List2D::addNewListItem(FJ_Base* item, int id)
{
    int index = 0;
    unsigned int r;
    for (r = 0; r < m_items.size(); ++r)
    {
        unsigned int c;
        for (c = 0; c < m_items[r].size(); ++c)
            ++index;
    }
    item->setId(id);
    item->setListId(index);
    item->setObjListParent(this);

    int newCount = index + 1;
    int itemWidth = static_cast<int>(item->getWidth());
    int itemSpacing = m_itemSpacing;
    int cellWidth = itemWidth + itemSpacing;
    int row;
    int rowStart;
    if (cellWidth * newCount - itemSpacing > Math::round(getWidth()))
    {
        row = index / m_itemsPerRow;
        rowStart = m_itemsPerRow * row;
    }
    else
    {
        row = index / newCount;
        rowStart = newCount * row;
        m_itemsPerRow = newCount;
    }
    int column = index - rowStart;

    if (column == 0)
        m_items.push_back(std::vector<FJ_FuiNode*>());
    m_items[row].push_back(item);

    item->m_navLeftName = m_navLeftName;
    item->m_navRightName = m_navRightName;
    item->m_navUpName = m_navUpName;
    item->m_navDownName = m_navDownName;

    if (column < 1)
    {
        if (row >= 1)
        {
            item->m_navLeftName = m_items[row - 1][m_itemsPerRow - 1]->getName();
            m_items[row - 1][m_itemsPerRow - 1]->asFJ_Base()->m_navRightName = item->getName();
        }
    }
    else
    {
        item->m_navLeftName = m_items[row][column - 1]->getName();
        m_items[row][column - 1]->asFJ_Base()->m_navRightName = item->getName();
    }

    if (row > 0)
    {
        unsigned int col;
        for (col = 0; col < m_items[row - 1].size(); ++col)
        {
            if (static_cast<int>(col) > column)
                m_items[row - 1][col]->asFJ_Base()->m_navDownName = item->getName();
        }
        item->m_navUpName = m_items[row - 1][column]->getName();
        m_items[row - 1][column]->asFJ_Base()->m_navDownName = item->getName();
    }

    item->addEventListener(eFJEventType_FocusIn, this, _ItemFocusInHandler, false);
    FJ_FuiNode* parent = getParent();
    parent->addChild(item);
    if (index == m_activeItem)
        SetListFocus();
    ScrollList(false, true, false);
    HandleScrollListArrows(true);
}

void FJ_List2D::removeAllItems()
{
    bool hadFocus = false;
    for (unsigned int row = 0; row < m_items.size(); ++row)
    {
        for (unsigned int column = 0; column < m_items[row].size(); ++column)
        {
            if (getStage()->getFocus() == m_items[row][column])
                hadFocus = true;
            if (m_items[row][column]->isFJ_Base())
            {
                for (unsigned int i = 0; i < m_items[row][column]->asFJ_Base()->m_objects.size(); ++i)
                    getParent()->removeChild(m_items[row][column]->asFJ_Base()->m_objects[i], true);
            }
            getParent()->removeChild(m_items[row][column], true);
        }
        m_items[row].clear();
    }
    m_items.clear();
    m_activeItem = 0;
    m_firstVisibleRow = 0;
    HandleScrollListArrows(false);
    if (hadFocus)
        getStage()->setFocus(this);
}

void FJ_List2D::EnableItem(int index, bool enabled)
{
    int row = index / m_itemsPerRow;
    int column = index - m_itemsPerRow * row;
    if (static_cast<unsigned int>(row) < m_items.size() && static_cast<unsigned int>(column) < m_items[row].size())
    {
        if (m_items[row][column]->isFJ_ListButtonAchievement())
            m_items[row][column]->asFJ_ListButtonAchievement()->EnableButton(enabled);
    }
}

void FJ_List2D::SetTouchFocus(int x, int y, bool focus)
{
    if (!focus)
    {
        unsigned int row;
        for (row = 0; row < m_items.size(); ++row)
        {
            unsigned int column;
            for (column = 0; column < m_items[row].size(); ++column)
            {
                if (m_items[row][column]->getY() < static_cast<float>(y)
                    && static_cast<float>(y) < m_items[row][column]->getY() + m_items[row][column]->getHeight()
                    && m_items[row][column]->getX() < static_cast<float>(x)
                    && static_cast<float>(x) < m_items[row][column]->getX() + m_items[row][column]->getWidth())
                {
                    m_touchRow = static_cast<int>(row);
                    m_touchFocus = false;
                    HighlightItem(static_cast<int>(row) * m_itemsPerRow + static_cast<int>(column));
                    break;
                }
            }
        }
        m_touchY = y;
        m_touchX = x;
    }
    else
    {
        if (!m_items.empty() && static_cast<unsigned int>(m_visibleRows) < m_items.size())
        {
            if (y < m_touchY)
            {
                int rows = (m_touchY - y) / (static_cast<int>(m_items[0][0]->getHeight()) + m_itemSpacing);
                if (rows > 0)
                {
                    int firstVisibleRow = m_firstVisibleRow + rows;
                    m_touchFocus = true;
                    if (static_cast<unsigned int>(firstVisibleRow + m_visibleRows) < m_items.size())
                        m_firstVisibleRow = firstVisibleRow;
                    else
                        m_firstVisibleRow = static_cast<int>(m_items.size()) - m_visibleRows;
                    ScrollList(false, false, true);
                    m_touchY = y;
                    if (m_downArrow != 0)
                        m_downArrow->ChangeState(4);
                    if (m_activeRow < m_firstVisibleRow)
                        HighlightItem(m_firstVisibleRow * m_itemsPerRow + m_activeColumn);
                }
            }
            else if (m_touchY < y)
            {
                int rows = (y - m_touchY) / (static_cast<int>(m_items[0][0]->getHeight()) + m_itemSpacing);
                if (rows > 0)
                {
                    int firstVisibleRow = m_firstVisibleRow - rows;
                    m_touchFocus = true;
                    if (firstVisibleRow < 0)
                        firstVisibleRow = 0;
                    m_firstVisibleRow = firstVisibleRow;
                    ScrollList(true, false, true);
                    m_touchY = y;
                    if (m_upArrow != 0)
                        m_upArrow->ChangeState(1);
                    int lastVisibleRow = m_firstVisibleRow + m_visibleRows - 1;
                    if (lastVisibleRow < m_activeRow)
                        HighlightItem(lastVisibleRow * m_itemsPerRow + m_activeColumn);
                }
            }
        }
    }
}

uint32_t FJ_List2D::CanTouchFocus(int, int)
{
    return 1;
}

uint32_t FJ_List2D::CanTouchTrigger(int x, int y)
{
    unsigned int row;
    for (row = 0; row < m_items.size(); ++row)
    {
        unsigned int column;
        for (column = 0; column < m_items[row].size(); ++column)
        {
            if (static_cast<float>(y) > m_items[row][column]->getY()
                && static_cast<float>(y) < m_items[row][column]->getY() + m_items[row][column]->getHeight()
                && static_cast<float>(x) > m_items[row][column]->getX()
                && static_cast<float>(x) < m_items[row][column]->getX() + m_items[row][column]->getWidth()
                && !m_touchFocus && m_touchRow == row)
                return 1;
        }
    }
    return 0;
}

void FJ_List2D::setAction(unsigned char action, char* value)
{
    switch (action)
    {
    case 10:
        m_outline = std::strcmp(value, "true") == 0;
        break;
    case 0x1d:
        m_itemSpacing = std::atoi(value);
        break;
    default:
        break;
    }
}
