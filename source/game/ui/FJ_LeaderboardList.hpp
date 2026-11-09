#ifndef GAME_UI_FJ_LEADERBOARDLIST_HPP
#define GAME_UI_FJ_LEADERBOARDLIST_HPP

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_Label;
class FJ_LeaderboardEntry;
class FJ_LeaderboardHeader;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_LeaderboardList : public FJ_FuiNode
{
    friend class fuiRenderNodeTimeline;

public:
    struct CLeaderboardEntry
    {
        int rank;
        std::wstring gamertag;
        bool singleValue;
        std::wstring values[7];
    };

    FJ_LeaderboardList(fuiRenderNode* renderNode);

    void SetupPleaseWait(FJ_Label* label);
    void UpdateLeaderboardCache(bool forward);
    void RequestData(bool forward);
    void addHeader();
    void HandleScrollListArrows(bool added);
    void addNewItem(int index);
    void UpdateVisibleLeaderboard();
    void NavigateLeaderboard(bool up, bool page);
    static FJ_FuiNode* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void ASConstructor();
    void SetupTitles(std::wstring rankTitle, std::wstring gamertagTitle);
    void SetDataLoading(bool loading);
    void InitLeaderboard(int focusRank, int totalEntries, int numColumns);
    void SetColumnIcon(int column, int icon);
    void ResetLeaderboard();
    void TouchHighlightEntry(int index);
    void SetTouchFocus(int x, int y, bool dragging);
    void FocusOnFirstEntry();
    void AddDataSet(bool update, int rank, int displayRank, const std::wstring& gamertag, bool singleValue,
                    const std::wstring* column1, const std::wstring* column2, const std::wstring* column3,
                    const std::wstring* column4, const std::wstring* column5, const std::wstring* column6,
                    const std::wstring* column7);

private:
    bool m_small;
    int m_entrySpacing;
    int m_visibleEntries;
    int m_maxCachedEntries;
    int m_requestSize;
    std::map<int, CLeaderboardEntry> m_leaderboard;
    int m_firstVisibleRank;
    int m_firstCachedRank;
    int m_lastCachedRank;
    int m_totalEntries;
    int m_numColumns;
    bool m_needsInit;
    bool m_dataLoading;
    std::vector<FJ_LeaderboardEntry*> m_entries;
    FJ_LeaderboardHeader* m_header;
    int m_highlightEntry;
    int m_focusRank;
    std::wstring m_rankTitle;
    std::wstring m_gamertagTitle;
    int m_columnIcons[7];
    FJ_ScrollArrow* m_upArrow;
    FJ_ScrollArrow* m_downArrow;
    FJ_Label* m_pleaseWait;
    unsigned int m_touchEntry;
    int m_touchY;
};

#endif
