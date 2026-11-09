#include "game/ui/FJ_LeaderboardList.hpp"

#include "game/ui/FJ_LeaderboardEntry.hpp"
#include "game/ui/FJ_Factory.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LeaderboardHeader.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

FJ_LeaderboardList::FJ_LeaderboardList(fuiRenderNode* renderNode)
    : FJ_FuiNode(renderNode),
      m_leaderboard(),
      m_entries(),
      m_rankTitle(),
      m_gamertagTitle()
{
    m_small = false;
    m_entrySpacing = 4;
    m_visibleEntries = 10;
    m_maxCachedEntries = 101;
    m_requestSize = 15;
    m_firstVisibleRank = 0;
    m_firstCachedRank = -1;
    m_lastCachedRank = -1;
    m_needsInit = true;
    m_dataLoading = true;
    m_header = NULL;
    m_highlightEntry = 0;
    m_touchEntry = 0;
    m_touchY = 0;
}

void FJ_LeaderboardList::SetupPleaseWait(FJ_Label* label)
{
    m_pleaseWait = label;
}

void FJ_LeaderboardList::UpdateLeaderboardCache(bool forward)
{
    if (!forward)
    {
        for (int i = 0; i < m_requestSize; ++i)
        {
            m_leaderboard.erase(m_lastCachedRank);
            --m_lastCachedRank;
        }
    }
    else
    {
        for (int i = 0; i < m_requestSize; ++i)
        {
            m_leaderboard.erase(m_firstCachedRank);
            ++m_firstCachedRank;
        }
    }
}

void FJ_LeaderboardList::RequestData(bool forward)
{
    if (!forward)
    {
        int first = m_firstCachedRank - m_requestSize;
        if (first < 0)
            first = 0;
        m_firstCachedRank = first;
        getRoot()->m_pCallbackScene->handleRequestMoreData(m_firstCachedRank, !forward);
    }
    else
    {
        int last = m_lastCachedRank + m_requestSize;
        if (m_totalEntries < last)
            last = m_totalEntries;
        m_lastCachedRank = last;
        getRoot()->m_pCallbackScene->handleRequestMoreData(m_lastCachedRank, !forward);
    }
    if (m_lastCachedRank - m_firstCachedRank >= m_maxCachedEntries)
        UpdateLeaderboardCache(forward);
}

void FJ_LeaderboardList::addHeader()
{
    if (m_small)
        m_header = static_cast<FJ_LeaderboardHeader*>(
            FJ_Factory::createObject("fourj.listClasses.FJ_LeaderboardHeaderSmall", m_pRenderNode, true, NULL));
    else
        m_header = static_cast<FJ_LeaderboardHeader*>(
            FJ_Factory::createObject("fourj.listClasses.FJ_LeaderboardHeaderNormal", m_pRenderNode, true, NULL));
    m_header->setWidth(getWidth());
    m_header->Init(m_numColumns, getParent(), getX(), getY());
    m_header->SetupHeaders(m_rankTitle, m_gamertagTitle);
    m_header->setX(getX() + Math::round((getWidth() - m_header->getWidth()) * 0.5f));
    m_header->setY(getY());
    getParent()->addChild(m_header);
    m_header->SetupSlot(0, m_columnIcons[0]);
    m_header->SetupSlot(1, m_columnIcons[1]);
    m_header->SetupSlot(2, m_columnIcons[2]);
    m_header->SetupSlot(3, m_columnIcons[3]);
    m_header->SetupSlot(4, m_columnIcons[4]);
    m_header->SetupSlot(5, m_columnIcons[5]);
    m_header->SetupSlot(6, m_columnIcons[6]);
}

void FJ_LeaderboardList::HandleScrollListArrows(bool added)
{
    if (added && m_entries.size() >= static_cast<unsigned int>(m_visibleEntries) && m_upArrow->getParent() == NULL)
    {
        getParent()->addChild(m_upArrow);
        getParent()->addChild(m_downArrow);
        m_upArrow->setX(m_entries[m_entries.size() - 1]->getX() + m_entries[m_entries.size() - 1]->getWidth()
                        - m_upArrow->getWidth() * 1.5f);
        m_upArrow->setY(m_entries[m_entries.size() - 1]->getY() + m_entries[m_entries.size() - 1]->getHeight()
                        + m_upArrow->getHeight() * 0.5f);
        m_downArrow->setX(m_entries[m_entries.size() - 1]->getX() + m_entries[m_entries.size() - 1]->getWidth()
                          - m_upArrow->getWidth() * 0.5f);
        m_downArrow->setY(m_upArrow->getY());
        m_upArrow->ChangeState(0);
        m_downArrow->ChangeState(3);
    }
    if (m_firstVisibleRank > 0 && m_totalEntries > m_visibleEntries)
        m_upArrow->setVisible(true);
    else
        m_upArrow->setVisible(false);
    if (m_firstVisibleRank < m_totalEntries - m_visibleEntries)
        m_downArrow->setVisible(true);
    else
        m_downArrow->setVisible(false);
}

void FJ_LeaderboardList::addNewItem(int index)
{
    FJ_LeaderboardEntry* entry;
    if (m_small)
        entry = static_cast<FJ_LeaderboardEntry*>(
            FJ_Factory::createObject("fourj.listClasses.FJ_LeaderboardEntrySmall", m_pRenderNode, true, NULL));
    else
        entry = static_cast<FJ_LeaderboardEntry*>(
            FJ_Factory::createObject("fourj.listClasses.FJ_LeaderboardEntryNormal", m_pRenderNode, true, NULL));
    entry->setWidth(getWidth());
    entry->setX(getX() + Math::round((getWidth() - entry->getWidth()) * 0.5f));
    entry->setY(getY() + m_header->getHeight() + m_entrySpacing + (entry->getHeight() + m_entrySpacing) * index);
    FJ_LeaderboardHeader* header = m_header;
    int iconHolderHalfWidth = header->getIconHolderHalfWidth();
    int firstEntryPos = header->getFirstEntryPos();
    int entrySpacing = header->getEntrySpacing();
    entry->Init(m_numColumns, firstEntryPos, entrySpacing, iconHolderHalfWidth);
    m_entries.push_back(entry);
    getParent()->addChild(entry);
    HandleScrollListArrows(true);
}

void FJ_LeaderboardList::UpdateVisibleLeaderboard()
{
    if (m_needsInit)
    {
        int count = m_totalEntries < m_visibleEntries ? m_totalEntries : m_visibleEntries;
        addHeader();
        for (int i = 0; i < count; ++i)
            addNewItem(i);
    }
    for (unsigned int i = 0; i < m_entries.size(); ++i)
    {
        if (m_leaderboard.find(m_firstVisibleRank + i) != m_leaderboard.end())
            m_entries[i]->SetData(&m_leaderboard[m_firstVisibleRank + i]);
        else
            m_entries[i]->SetData(NULL);
    }
}

void FJ_LeaderboardList::NavigateLeaderboard(bool up, bool page)
{
    if (m_entries.size() == 0)
        return;

    bool rankChanged = true;
    if (up)
    {
        int highlight;
        if (page)
        {
            highlight = -1;
            if (m_highlightEntry > 0)
                highlight = 0;
        }
        else
            highlight = m_highlightEntry - 1;

        if (highlight >= 0)
        {
            m_highlightEntry = highlight;
        }
        else
        {
            if (m_firstVisibleRank == 0)
                rankChanged = false;
            m_highlightEntry = 0;
            if (m_visibleEntries < m_totalEntries)
            {
                int firstRank;
                if (page)
                {
                    firstRank = m_firstVisibleRank - m_visibleEntries;
                    if (firstRank < 0)
                        firstRank = 0;
                }
                else
                {
                    firstRank = m_firstVisibleRank - 1;
                }

                if (firstRank < 0)
                {
                    m_firstVisibleRank = 0;
                }
                else
                {
                    m_firstVisibleRank = firstRank;
                    if (firstRank < m_firstCachedRank)
                        RequestData(false);
                    UpdateVisibleLeaderboard();
                    if (m_upArrow != NULL)
                        m_upArrow->ChangeState(1);
                }
            }
        }
    }
    else
    {
        unsigned int highlight;
        if (page)
        {
            highlight = m_entries.size();
            if (static_cast<unsigned int>(m_highlightEntry) < m_entries.size() - 1)
                highlight = m_entries.size() - 1;
        }
        else
            highlight = m_highlightEntry + 1;

        if (highlight < m_entries.size())
        {
            m_highlightEntry = highlight;
        }
        else
        {
            int lastFirstRank = m_totalEntries - m_visibleEntries;
            if (m_firstVisibleRank == lastFirstRank)
                rankChanged = false;
            m_highlightEntry = m_entries.size() - 1;
            if (m_visibleEntries < m_totalEntries)
            {
                int firstRank;
                if (page)
                {
                    firstRank = m_firstVisibleRank + m_visibleEntries;
                    if (m_totalEntries < m_firstVisibleRank + m_visibleEntries + m_visibleEntries)
                        firstRank = lastFirstRank;
                }
                else
                    firstRank = m_firstVisibleRank + 1;

                if (firstRank + m_visibleEntries > m_totalEntries)
                {
                    m_firstVisibleRank = lastFirstRank;
                }
                else
                {
                    m_firstVisibleRank = firstRank;
                    if (m_lastCachedRank < static_cast<int>(firstRank + (m_entries.size() - 1)))
                        RequestData(true);
                    UpdateVisibleLeaderboard();
                    if (m_downArrow != NULL)
                        m_downArrow->ChangeState(4);
                }
            }
        }
    }

    if (rankChanged)
        getRoot()->m_pCallbackScene->handleSelectionChanged(m_firstVisibleRank + m_highlightEntry);

    if (getStage()->getFocus() != m_entries[m_highlightEntry])
    {
        getStage()->setFocus(m_entries[m_highlightEntry]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
    }
    HandleScrollListArrows(false);
}

FJ_FuiNode* FJ_LeaderboardList::Create(fuiRenderNode* renderNode)
{
    return new FJ_LeaderboardList(renderNode);
}

unsigned int FJ_LeaderboardList::getType()
{
    return 104;
}

void FJ_LeaderboardList::ASConstructor()
{
    m_upArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    m_downArrow = static_cast<FJ_ScrollArrow*>(
        FJ_Factory::createObject("fourj.FJ_ScrollArrow", m_pRenderNode, true, NULL));
    FJ_FuiNode* background = getChildByName(std::string("Background"))->asMovieClip();
    background->setVisible(false);
}

void FJ_LeaderboardList::SetupTitles(std::wstring rankTitle, std::wstring gamertagTitle)
{
    m_rankTitle = rankTitle;
    m_gamertagTitle = gamertagTitle;
}

void FJ_LeaderboardList::SetDataLoading(bool loading)
{
    m_dataLoading = loading;
    m_pleaseWait->setVisible(loading);
}

void FJ_LeaderboardList::InitLeaderboard(int focusRank, int totalEntries, int numColumns)
{
    m_focusRank = focusRank;
    m_totalEntries = totalEntries;
    m_numColumns = numColumns;
    SetDataLoading(true);
}

void FJ_LeaderboardList::SetColumnIcon(int column, int icon)
{
    m_columnIcons[column] = icon;
}

void FJ_LeaderboardList::ResetLeaderboard()
{
    if (m_header != NULL && m_header->getParent() != NULL)
    {
        m_header->Destroy();
        m_header->getParent()->removeChild(m_header, true);
        m_header = NULL;
    }
    for (unsigned int i = 0; i < m_entries.size(); ++i)
    {
        if (m_entries[i] != NULL && m_entries[i]->getParent() != NULL)
            m_entries[i]->getParent()->removeChild(m_entries[i], true);
    }
    m_upArrow->setVisible(false);
    m_downArrow->setVisible(false);
    m_entries.clear();
    m_leaderboard.clear();
    SetDataLoading(true);
    m_firstVisibleRank = 0;
    m_highlightEntry = 0;
    m_firstCachedRank = -1;
    m_needsInit = true;
    m_lastCachedRank = -1;
}

void FJ_LeaderboardList::TouchHighlightEntry(int index)
{
    if (static_cast<unsigned int>(index) < m_entries.size())
    {
        if (index != m_highlightEntry)
        {
            m_highlightEntry = index;
            getStage()->setFocus(m_entries[index]);
            getRoot()->m_pCallbackScene->handleSelectionChanged(m_firstVisibleRank + m_highlightEntry);
        }
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
    }
}

void FJ_LeaderboardList::SetTouchFocus(int x, int y, bool dragging)
{
    if (!dragging)
    {
        for (unsigned int i = 0; i < m_entries.size(); ++i)
        {
            if (m_entries[i]->getY() < y && y < m_entries[i]->getY() + m_entries[i]->getHeight())
            {
                m_touchEntry = i;
                TouchHighlightEntry(i);
                m_touchY = y;
                return;
            }
        }
        m_touchY = y;
        return;
    }

    if (m_entries.empty() || static_cast<unsigned int>(m_totalEntries) <= m_entries.size())
        return;

    if (y < m_touchY)
    {
        int rows = (m_touchY - y) / (m_entries[0]->getHeight() + m_entrySpacing);
        if (rows > 0)
        {
            int previousRank = m_firstVisibleRank;
            int firstRank = previousRank + rows;
            if (firstRank + m_visibleEntries > m_totalEntries)
                firstRank = m_totalEntries - m_visibleEntries;
            m_firstVisibleRank = firstRank;
            if (previousRank != firstRank)
            {
                if (static_cast<unsigned int>(m_lastCachedRank) < firstRank + m_entries.size() - 1)
                    RequestData(true);
                UpdateVisibleLeaderboard();
                if (m_downArrow != NULL)
                    m_downArrow->ChangeState(4);
                int highlight = m_highlightEntry - rows;
                if (highlight < 0)
                    highlight = 0;
                m_highlightEntry = highlight;
                getStage()->setFocus(m_entries[highlight]);
            }
            m_touchY = y;
        }
    }
    else if (m_touchY < y)
    {
        int rows = (y - m_touchY) / (m_entries[0]->getHeight() + m_entrySpacing);
        if (rows >= 1)
        {
            int previousRank = m_firstVisibleRank;
            m_firstVisibleRank = previousRank - rows < 0 ? 0 : previousRank - rows;
            if (previousRank != m_firstVisibleRank)
            {
                if (m_firstVisibleRank < m_firstCachedRank)
                    RequestData(false);
                UpdateVisibleLeaderboard();
                if (m_upArrow != NULL)
                    m_upArrow->ChangeState(1);
                unsigned int highlight = m_highlightEntry + rows;
                if (highlight < m_entries.size())
                    m_highlightEntry = highlight;
                else
                    m_highlightEntry = m_entries.size() - 1;
                getStage()->setFocus(m_entries[m_highlightEntry]);
            }
            m_touchY = y;
        }
    }
    HandleScrollListArrows(false);
}

void FJ_LeaderboardList::FocusOnFirstEntry()
{
    unsigned int index = m_focusRank - m_firstVisibleRank;
    if (static_cast<int>(index) >= m_visibleEntries
        && static_cast<int>(index) < m_lastCachedRank - m_firstCachedRank + 1)
    {
        m_firstVisibleRank += m_focusRank - m_firstCachedRank + 1 - m_visibleEntries;
        index = m_focusRank - m_firstVisibleRank;
    }
    if (index < m_entries.size())
    {
        m_highlightEntry = index;
        getStage()->setFocus(m_entries[index]);
        dispatchEvent(new FJ_Event(eFJEventType_FocusChangeUser, true, false));
    }
}

void FJ_LeaderboardList::AddDataSet(bool update, int rank, int displayRank, const std::wstring& gamertag,
                                    bool singleValue, const std::wstring* column1, const std::wstring* column2,
                                    const std::wstring* column3, const std::wstring* column4,
                                    const std::wstring* column5, const std::wstring* column6,
                                    const std::wstring* column7)
{
    CLeaderboardEntry entry;
    std::wstring name = gamertag;
    if (gamertag.size() > 16)
        name = gamertag.substr(0, 16) + L"...";
    entry.rank = displayRank;
    entry.gamertag = name;
    entry.singleValue = singleValue;
    entry.values[0] = column1 != NULL ? *column1 : L"";
    entry.values[1] = column2 != NULL ? *column2 : L"";
    entry.values[2] = column3 != NULL ? *column3 : L"";
    entry.values[3] = column4 != NULL ? *column4 : L"";
    entry.values[4] = column5 != NULL ? *column5 : L"";
    entry.values[5] = column6 != NULL ? *column6 : L"";
    entry.values[6] = column7 != NULL ? *column7 : L"";
    m_leaderboard[rank] = entry;

    if (m_needsInit)
    {
        if (m_firstCachedRank < 0 || rank < m_firstCachedRank)
            m_firstCachedRank = rank;
        if (m_lastCachedRank < 0 || m_lastCachedRank < rank)
            m_lastCachedRank = rank;
        m_firstVisibleRank = m_firstCachedRank;
    }

    if (update)
    {
        UpdateVisibleLeaderboard();
        if (m_needsInit)
            FocusOnFirstEntry();
        m_needsInit = false;
        SetDataLoading(false);
    }
}
