#ifndef GAME_UI_UISCENE_LEADERBOARDSMENU_HPP
#define GAME_UI_UISCENE_LEADERBOARDSMENU_HPP

#include <string>
#include <vector>

#include "game/core/LeaderboardInterface.hpp"
#include "game/network/LeaderboardReadListener.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_LeaderboardList.hpp"
#include "game/ui/UIScene.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/LevelRules.hpp"
#include "platform/wiiu/C4JStorage.hpp"

class UILayer;

class UIScene_LeaderboardsMenu : public UIScene, public LeaderboardReadListener
{
public:
    enum
    {
        MAX_STAT_COLUMNS = 7,
        MAX_LOADED_ENTRIES = 0x65,
        ENTRIES_PER_PAGE = 15,
        STAT_TEXT_LENGTH = 12,
        SCORE_TEXT_LENGTH = 12,
        GAMERTAG_TEXT_LENGTH = 34,
        GAMERTAG_DISPLAY_LENGTH = 17,
        MAX_DISPLAYED_SCORE = 9999999,
        MAX_DISPLAYED_STAT = 99999,
        DEBUG_MASK_FAKE_LEADERBOARD = 0x10000,
        LEADERBOARD_ICON_COUNT = 28,
        ICON_ID_32002 = 32002,
        ICON_ID_32003 = 32003,
        ICON_ID_32005 = 32005,
        ICON_ID_32006 = 32006,
        ICON_ID_32008 = 32008,
        ICON_ID_32009 = 32009,
        ICON_ID_32010 = 32010,
        ICON_ID_32011 = 32011,
        ICON_ID_32012 = 32012
    };

    struct LeaderboardDescriptor
    {
        unsigned int numStats;
        unsigned int id;
        unsigned int themeId;
    };

    struct LeaderboardEntry
    {
        PlayerUID uid;
        unsigned int rank;
        unsigned int score;
        wchar_t scoreText[SCORE_TEXT_LENGTH];
        wchar_t gamertag[GAMERTAG_TEXT_LENGTH];
        unsigned int stats[MAX_STAT_COLUMNS];
        wchar_t statText[MAX_STAT_COLUMNS][STAT_TEXT_LENGTH];
        bool complete;
        bool selected;
        bool loading;
        bool visible;
        int messageStringId;
    };

    UIScene_LeaderboardsMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_LeaderboardsMenu();
    virtual bool OnStatsReadComplete(LeaderboardManager::eStatsReturn result, int userIndex,
                                     LeaderboardManager::ReadView readView);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void tick();
    virtual void handleTimerComplete(int timerId);
    virtual bool hasFocus(int sceneType);
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual void handleSelectionChanged(int selection);
    virtual void handleRequestMoreData(int id, bool forward);

    unsigned int GetEntryStartIndex();
    bool LanguageUsesCommaRadix(int language);
    void CopyLeaderboardEntry(LeaderboardManager::ReadScore* score, int index,
                              LeaderboardManager::EStatsType statsType);
    bool RetrieveStats();
    void SetLeaderboardHeader();
    void ReadStats(int startRank);
    int SetLeaderboardTitleIcons();
    void PopulateLeaderboard(LeaderboardManager::eStatsReturn result);
    void finishUpdatingLeaderboards();
    static void staticCtor();
    static int ExitLeaderboards(void* context, int pad, C4JStorage::EMessageResult result);

private:
    unsigned int m_visibleRowCount;
    std::vector<LeaderboardEntry> m_entries;
    unsigned int m_numStats;
    int m_statsType;
    int m_readType;
    unsigned int m_leaderboardIndex;
    int m_startRank;
    int m_numRows;
    unsigned int m_loadedRowCount;
    int m_visibleStartRank;
    int m_selectedEntry;
    bool m_leaderboardPopulated;
    bool m_statsIconsValid;
    LeaderboardInterface m_leaderboardInterface;
    UIControl_LeaderboardList m_leaderboardList;
    UIControl_Label m_filterLabel;
    UIControl_Label m_leaderboardLabel;
    UIControl_Label m_entriesLabel;
    UIControl_Label m_infoLabel;
    UIControl m_tabNavigation;
    int m_totalRows;
    LeaderboardManager::ReadView m_readView;
    bool m_loading;
    std::vector<LevelRules::LevelRulesetGroup*> m_levelRuleGroups;
    unsigned int m_result;
};

#endif
