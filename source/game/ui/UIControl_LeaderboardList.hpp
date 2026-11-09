#ifndef GAME_UI_UICONTROL_LEADERBOARDLIST_HPP
#define GAME_UI_UICONTROL_LEADERBOARDLIST_HPP

#include "game/ui/UIControl_Base.hpp"

class UIControl_LeaderboardList : public UIControl_Base
{
public:
    UIControl_LeaderboardList();
    virtual ~UIControl_LeaderboardList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    void clearList();
    void init(int controlId);
    void setupTitles(const std::wstring& title, const std::wstring& subtitle);
    void initLeaderboard(int focusRank, int totalEntries, int numColumns);
    void setColumnIcon(int column, int icon);
    void addDataSet(bool loading, int rank, int value, const std::wstring& gamertag, bool complete,
                    const std::wstring& firstColumnText, const std::wstring& secondColumnText,
                    const std::wstring& thirdColumnText, const std::wstring& fourthColumnText,
                    const std::wstring& fifthColumnText, const std::wstring& sixthColumnText,
                    const std::wstring& seventhColumnText);
    void SetTouchFocus(int x, int y, bool focus);

private:
    bool m_touchBoxAdded;
};

#endif
