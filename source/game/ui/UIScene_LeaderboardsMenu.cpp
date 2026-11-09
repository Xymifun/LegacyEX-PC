#include "game/ui/UIScene_LeaderboardsMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cwchar>
#include <cstring>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/XLanguage.hpp"


static std::vector<UIScene_LeaderboardsMenu::LeaderboardDescriptor> gLeaderboardDescriptors[4];
static int gLeaderboardIconIds[UIScene_LeaderboardsMenu::LEADERBOARD_ICON_COUNT];

UIScene_LeaderboardsMenu::UIScene_LeaderboardsMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer),
      m_leaderboardInterface(g_pLeaderboardManager)
{
    initialiseMovie();
    m_selectedEntry = -1;
    m_leaderboardPopulated = false;
    m_statsIconsValid = false;
    m_visibleStartRank = -1;
    m_loading = true;
    gApp.SetLiveLinkRequired(true);
    m_leaderboardIndex = 0;
    m_statsType = 0;
    SetLeaderboardHeader();
    m_readType = 0;
    wchar_t filterText[40];
    swprintf(filterText, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
             L10N::GetString(IDS_LEADERBOARD_FILTER_FRIENDS));
    m_filterLabel.init(UIString(filterText));
    wchar_t entriesText[40];
    swprintf(entriesText, 40, L"%ls%i", L10N::GetString(IDS_LEADERBOARD_ENTRIES), 0);
    m_entriesLabel.init(UIString(entriesText));
    ReadStats(-1);
    addTimer(0, 100);
    if (gApp.StartInstallDLCProcess(gProfile->GetPrimaryPad()) == 0)
        gApp.getLevelRules()->getRulesByType(LevelRules::eLevelRuleType_Glide, &m_levelRuleGroups, false);
    m_result = static_cast<unsigned int>(-1);
}

UIScene_LeaderboardsMenu::~UIScene_LeaderboardsMenu()
{
    gApp.SetLiveLinkRequired(false);
}

bool UIScene_LeaderboardsMenu::OnStatsReadComplete(LeaderboardManager::eStatsReturn result, int userIndex,
                                                   LeaderboardManager::ReadView readView)
{
    m_totalRows = userIndex;
    m_readView = readView;
    m_result = result;
    RetrieveStats();
    return true;
}

unsigned int UIScene_LeaderboardsMenu::GetEntryStartIndex()
{
    if (m_entries.size() == 0)
        return 0;
    return m_entries[0].rank;
}

bool UIScene_LeaderboardsMenu::LanguageUsesCommaRadix(int language)
{
    return language == XC_LANGUAGE_GERMAN || language == XC_LANGUAGE_FRENCH || language == XC_LANGUAGE_SPANISH
           || language == XC_LANGUAGE_PORTUGUESE || language == XC_LANGUAGE_POLISH || language == XC_LANGUAGE_RUSSIAN
           || language == XC_LANGUAGE_SWEDISH || language == XC_LANGUAGE_NORWEGIAN || language == XC_LANGUAGE_DUTCH
           || language == XC_LANGUAGE_FINNISH || language == XC_LANGUAGE_GREEK || language == XC_LANGUAGE_DANISH
           || language == XC_LANGUAGE_CZECH || language == XC_LANGUAGE_SLOVAK;
}

void UIScene_LeaderboardsMenu::CopyLeaderboardEntry(LeaderboardManager::ReadScore* score, int index,
                                                    LeaderboardManager::EStatsType statsType)
{
    LeaderboardEntry& entry = m_entries[index];
    memset(&entry, 0, sizeof(entry));
    entry.uid = score->m_uid;
    entry.score = score->m_uiRank;
    unsigned int displayedScore = entry.score;
    if (displayedScore > MAX_DISPLAYED_SCORE)
        displayedScore = MAX_DISPLAYED_SCORE;
    swprintf(entry.scoreText, SCORE_TEXT_LENGTH, L"%u", displayedScore);
    entry.messageStringId = score->m_iMessageStringId;
    if (m_readType != LeaderboardManager::eReadType_Friends)
    {
        entry.rank = entry.score - 1;
        if (index >= 1 && entry.rank <= m_entries[index - 1].rank)
            entry.rank = m_entries[index - 1].rank + 1;
    }
    else
    {
        entry.rank = static_cast<unsigned int>(index);
    }
    memcpy(entry.gamertag, score->m_wsGamertag.c_str(), score->m_wsGamertag.size() * sizeof(wchar_t));
    for (unsigned int i = 0; i < score->m_usNumStats; ++i)
    {
        entry.stats[i] = score->m_auiStats[i];
        memset(entry.statText[i], 0, sizeof(entry.statText[i]));
        if ((i == 0 && statsType == LeaderboardManager::eStatsType_TimePlayed)
            || (i == 2 && statsType == LeaderboardManager::eStatsType_DistanceTravelled))
        {
            unsigned int value = entry.stats[i];
            unsigned int millis = value % 1000;
            unsigned int totalSeconds = (value - millis) / 1000;
            unsigned int seconds = totalSeconds % 60;
            unsigned int minutes = (totalSeconds - seconds) / 60;
            swprintf(entry.statText[i], STAT_TEXT_LENGTH, L"%i:%i%i.%i%i%i", minutes % 60, (seconds / 10) % 10,
                     seconds % 10, (millis / 100) % 10, (millis / 10) % 10, millis % 10);
        }
        else if (statsType == LeaderboardManager::eStatsType_Score)
        {
            int digits = 0;
            for (unsigned int remaining = entry.stats[i]; remaining != 0; remaining /= 10)
                ++digits;
            if (digits < 4)
            {
                swprintf(entry.statText[i], STAT_TEXT_LENGTH, L"%um", entry.stats[i]);
            }
            else if (digits < 8)
            {
                if (!LanguageUsesCommaRadix(XGetLanguage()))
                {
                    swprintf(entry.statText[i], STAT_TEXT_LENGTH, L"%.1fkm",
                             static_cast<float>(entry.stats[i]) / 1000.0f);
                }
                else
                {
                    wchar_t* text = new wchar_t[STAT_TEXT_LENGTH];
                    swprintf(text, STAT_TEXT_LENGTH, L"%.1fkm", static_cast<float>(entry.stats[i]) / 1000.0f);
                    for (int k = 0; k < STAT_TEXT_LENGTH; ++k)
                    {
                        if (text[k] == L'.')
                            text[k] = L',';
                    }
                    swprintf(entry.statText[i], STAT_TEXT_LENGTH, text);
                    delete[] text;
                }
            }
            else
            {
                swprintf(entry.statText[i], STAT_TEXT_LENGTH, L"%.0fkm", static_cast<float>(entry.stats[i]) / 1000.0f);
            }
        }
        else
        {
            unsigned int displayedStat = entry.stats[i];
            if (displayedStat > MAX_DISPLAYED_STAT)
                displayedStat = MAX_DISPLAYED_STAT;
            swprintf(entry.statText[i], STAT_TEXT_LENGTH, L"%u", displayedStat);
        }
    }
}

bool UIScene_LeaderboardsMenu::RetrieveStats()
{
    if (gApp.DebugSettingsOn()
        && (GameSettings::GetGameSettingsDebugMask(-1, false) & DEBUG_MASK_FAKE_LEADERBOARD) != 0)
    {
        m_visibleRowCount = MAX_LOADED_ENTRIES;
        m_numStats = gLeaderboardDescriptors[m_statsType][m_leaderboardIndex].numStats;
        for (unsigned int rank = 0; rank < MAX_LOADED_ENTRIES; ++rank)
        {
            LeaderboardEntry entry = LeaderboardEntry();
            m_entries.push_back(entry);
            m_entries[rank].uid = PlayerUID(0);
            m_entries[rank].rank = rank;
            m_entries[rank].score = rank + 1;
            swprintf(m_entries[rank].scoreText, SCORE_TEXT_LENGTH, L"12345678");
            swprintf(m_entries[rank].gamertag, GAMERTAG_DISPLAY_LENGTH, L"WWWWWWWWWWWWWWWW");
            for (unsigned int column = 0; column < m_numStats; ++column)
            {
                if (m_statsType != LeaderboardManager::eStatsType_Score)
                {
                    m_entries[rank].stats[column] = 0xffff;
                    swprintf(m_entries[rank].statText[column], STAT_TEXT_LENGTH, L"%u", m_entries[rank].stats[column]);
                }
                else
                {
                    m_entries[rank].stats[column] = 0xffffffff;
                    if (!LanguageUsesCommaRadix(XGetLanguage()))
                    {
                        swprintf(m_entries[rank].statText[column], STAT_TEXT_LENGTH, L"%.1fkm",
                                 static_cast<float>(m_entries[rank].stats[column]) / 100.0f / 1000.0f);
                    }
                    else
                    {
                        wchar_t* text = new wchar_t[STAT_TEXT_LENGTH];
                        swprintf(text, STAT_TEXT_LENGTH, L"%.1fkm",
                                 static_cast<float>(m_entries[rank].stats[column]) / 100.0f / 1000.0f);
                        for (int k = 0; k < STAT_TEXT_LENGTH; ++k)
                        {
                            if (text[k] == L'.')
                                text[k] = L',';
                        }
                        swprintf(m_entries[rank].statText[column], STAT_TEXT_LENGTH, text);
                        delete[] text;
                    }
                }
            }
            m_entries[rank].complete = (rank == 0);
            m_entries[rank].selected = (rank != 0);
            m_entries[rank].loading = (rank != 0);
            m_entries[rank].visible = false;
        }
        m_startRank = 0;
        m_loadedRowCount = MAX_LOADED_ENTRIES;
        return true;
    }

    unsigned int numScores = m_readView.m_uiNumScores;
    m_loadedRowCount = numScores;
    if (m_visibleRowCount == 0)
    {
        m_entries.clear();
        if (m_readType == LeaderboardManager::eReadType_Friends)
            m_visibleRowCount = numScores;
        else
            m_visibleRowCount = static_cast<unsigned int>(m_totalRows);
        if (m_visibleRowCount == 0 || numScores == 0)
            return false;
        m_numStats = m_readView.m_pScores[0].m_usNumStats;
        unsigned int i = 0;
        do
        {
            LeaderboardEntry entry = LeaderboardEntry();
            m_entries.push_back(entry);
            CopyLeaderboardEntry(&m_readView.m_pScores[i], static_cast<int>(i),
                                 static_cast<LeaderboardManager::EStatsType>(m_statsType));
            ++i;
        } while (i < m_loadedRowCount);
        m_startRank = 0;
        m_visibleStartRank = -1;
        m_selectedEntry = -1;
        unsigned int selection;
        if (m_readType == LeaderboardManager::eReadType_MyScore)
        {
            unsigned int firstRank = GetEntryStartIndex();
            m_visibleStartRank = static_cast<int>(firstRank);
            unsigned int startRank = GetEntryStartIndex();
            unsigned int selectedRank = startRank;
            for (;; ++selectedRank)
            {
                if (GetEntryStartIndex() + m_entries.size() <= selectedRank)
                {
                    selection = startRank;
                    break;
                }
                if (m_entries[selectedRank - GetEntryStartIndex()].complete)
                {
                    m_selectedEntry = static_cast<int>(selectedRank);
                    if (static_cast<int>(selectedRank - firstRank) > 9)
                        m_visibleStartRank = static_cast<int>(selectedRank - 9);
                    if (static_cast<int>(selectedRank) >= 0)
                        return true;
                    selection = GetEntryStartIndex();
                    break;
                }
            }
        }
        else
        {
            selection = GetEntryStartIndex();
        }
        m_visibleStartRank = m_selectedEntry = static_cast<int>(selection);
        return true;
    }

    unsigned int previousStart = static_cast<unsigned int>(m_startRank);
    unsigned int readCount = numScores;
    unsigned int firstVisible = GetEntryStartIndex();
    if (previousStart < firstVisible && previousStart == 1)
    {
        readCount = firstVisible;
        m_loadedRowCount = firstVisible;
        firstVisible = GetEntryStartIndex();
    }

    bool trimFront = false;
    bool trimBack = false;
    const bool overCapacity = m_entries.size() + readCount >= MAX_LOADED_ENTRIES;
    unsigned int insertRank = 0;
    if (previousStart < firstVisible)
    {
        if (overCapacity)
            trimBack = true;
    }
    else
    {
        insertRank = static_cast<unsigned int>(m_entries.size());
        if (overCapacity)
            trimFront = true;
    }
    m_startRank = static_cast<int>(insertRank);

    for (unsigned int i = 0; i < m_loadedRowCount; ++i)
    {
        LeaderboardEntry entry = LeaderboardEntry();
        m_entries.insert(m_entries.begin() + insertRank + i, entry);
        CopyLeaderboardEntry(&m_readView.m_pScores[i], static_cast<int>(insertRank + i),
                             static_cast<LeaderboardManager::EStatsType>(m_statsType));
    }

    if (trimFront)
    {
        m_entries.erase(m_entries.begin(), m_entries.begin() + ENTRIES_PER_PAGE);
        m_startRank -= m_numRows;
    }
    else if (trimBack)
    {
        m_entries.erase(m_entries.end() - ENTRIES_PER_PAGE, m_entries.end());
    }
    return true;
}

void UIScene_LeaderboardsMenu::SetLeaderboardHeader()
{
    const LeaderboardDescriptor& descriptor = gLeaderboardDescriptors[m_statsType][m_leaderboardIndex];
    std::wstring label = L10N::GetString(descriptor.id);
    if (m_statsType == LeaderboardManager::eStatsType_TimePlayed
        || m_statsType == LeaderboardManager::eStatsType_DistanceTravelled)
    {
        unsigned int uniqueId = LevelGenerationOptions::CreateUniqueId(static_cast<int>(descriptor.themeId), 0);
        if (m_levelRuleGroups.size() == 0)
            gApp.getLevelRules()->getRulesByType(LevelRules::eLevelRuleType_Glide, &m_levelRuleGroups, false);
        for (std::vector<LevelRules::LevelRulesetGroup*>::iterator group = m_levelRuleGroups.begin();
             group != m_levelRuleGroups.end(); ++group)
        {
            LevelGenerationOptions* options = (*group)->rulesets.front()->getGenerationOptions();
            if (options->getUniqueId() == uniqueId)
            {
                label = label + L" : " + options->getWorldName();
                break;
            }
        }
    }
    m_leaderboardLabel.setLabel(UIString(label), false, false);
}

void UIScene_LeaderboardsMenu::ReadStats(int startRank)
{
    m_result = static_cast<unsigned int>(-1);
    if (startRank == -1)
    {
        m_startRank = 1;
        m_numRows = 0xf;
        m_visibleRowCount = 0;
        m_loadedRowCount = 0;
        m_leaderboardList.clearList();
    }
    else
    {
        m_startRank = startRank;
    }

    int readType = m_readType;
    if ((readType == LeaderboardManager::eReadType_MyScore || readType == LeaderboardManager::eReadType_TopRank)
        && startRank != -1)
    {
        readType = LeaderboardManager::eReadType_TopRank;
    }

    switch (readType)
    {
    case LeaderboardManager::eReadType_Friends:
    {
        PlayerUID uid(0);
        gProfile->GetXUID(gProfile->GetPrimaryPad(), &uid, true);
        m_leaderboardInterface.ReadStats_Friends(
            this, m_leaderboardIndex, static_cast<LeaderboardManager::EStatsType>(m_statsType), uid,
            static_cast<unsigned int>(m_startRank), static_cast<unsigned int>(m_numRows));
        break;
    }
    case LeaderboardManager::eReadType_MyScore:
    {
        PlayerUID uid(0);
        gProfile->GetXUID(gProfile->GetPrimaryPad(), &uid, true);
        m_leaderboardInterface.ReadStats_MyScore(this, m_leaderboardIndex,
                                                 static_cast<LeaderboardManager::EStatsType>(m_statsType), uid,
                                                 static_cast<unsigned int>(m_numRows));
        break;
    }
    case LeaderboardManager::eReadType_TopRank:
        m_leaderboardInterface.ReadStats_TopRank(
            this, m_leaderboardIndex, static_cast<LeaderboardManager::EStatsType>(m_statsType),
            static_cast<unsigned int>(m_startRank), static_cast<unsigned int>(m_numRows));
        break;
    }
    m_infoLabel.setLabel(UIString(L10N::GetString(IDS_LEADERBOARD_LOADING)), false, false);
    m_infoLabel.setVisible(true);
}

void UIScene_LeaderboardsMenu::staticCtor()
{
    int iconIds[LEADERBOARD_ICON_COUNT] = {ICON_ID_32003,
                                           ICON_ID_32002,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           0,
                                           ICON_ID_32005,
                                           ICON_ID_32009,
                                           ICON_ID_32008,
                                           ICON_ID_32010,
                                           ICON_ID_32011,
                                           ICON_ID_32006,
                                           ICON_ID_32012};
    iconIds[2] = Item::getId(Items::minecart);
    iconIds[3] = Item::getId(Items::boat);
    iconIds[7] = Block::getId(Blocks::dirt);
    iconIds[8] = Block::getId(Blocks::cobblestone);
    iconIds[9] = Block::getId(Blocks::sand);
    iconIds[10] = Block::getId(Blocks::stone);
    iconIds[11] = Block::getId(Blocks::gravel);
    iconIds[12] = Block::getId(Blocks::clay);
    iconIds[13] = Block::getId(Blocks::obsidian);
    iconIds[14] = Item::getId(Items::egg);
    iconIds[15] = Item::getId(Items::wheat);
    iconIds[16] = Block::getId(Blocks::brown_mushroom);
    iconIds[17] = Item::getId(Items::reeds);
    iconIds[18] = Item::getId(Items::milkBucket);
    iconIds[19] = Block::getId(Blocks::pumpkin);
    memcpy(gLeaderboardIconIds, iconIds, sizeof(iconIds));

    LeaderboardDescriptor descriptor = {4, IDS_LEADERBOARD_TRAVELLING_PEACEFUL, 0};
    gLeaderboardDescriptors[0].push_back(descriptor);
    descriptor.numStats = 4;
    descriptor.id = IDS_LEADERBOARD_TRAVELLING_EASY;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[0].push_back(descriptor);
    descriptor.numStats = 4;
    descriptor.id = IDS_LEADERBOARD_TRAVELLING_NORMAL;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[0].push_back(descriptor);
    descriptor.numStats = 4;
    descriptor.id = IDS_LEADERBOARD_TRAVELLING_HARD;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[0].push_back(descriptor);

    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_MINING_BLOCKS_PEACEFUL;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[1].push_back(descriptor);
    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_MINING_BLOCKS_EASY;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[1].push_back(descriptor);
    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_MINING_BLOCKS_NORMAL;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[1].push_back(descriptor);
    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_MINING_BLOCKS_HARD;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[1].push_back(descriptor);

    descriptor.numStats = 6;
    descriptor.id = IDS_LEADERBOARD_FARMING_PEACEFUL;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[2].push_back(descriptor);
    descriptor.numStats = 6;
    descriptor.id = IDS_LEADERBOARD_FARMING_EASY;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[2].push_back(descriptor);
    descriptor.numStats = 6;
    descriptor.id = IDS_LEADERBOARD_FARMING_NORMAL;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[2].push_back(descriptor);
    descriptor.numStats = 6;
    descriptor.id = IDS_LEADERBOARD_FARMING_HARD;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[2].push_back(descriptor);

    descriptor.numStats = 0;
    descriptor.id = 0xffffffff;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[3].push_back(descriptor);
    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_KILLS_EASY;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[3].push_back(descriptor);
    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_KILLS_NORMAL;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[3].push_back(descriptor);
    descriptor.numStats = 7;
    descriptor.id = IDS_LEADERBOARD_KILLS_HARD;
    descriptor.themeId = 0;
    gLeaderboardDescriptors[3].push_back(descriptor);
}

unsigned int UIScene_LeaderboardsMenu::mapElementsAndNames()
{
    fuiRenderNode* currentNode = m_movie->getRootNode();
    {
        m_leaderboardList.setupControl(this, currentNode, std::string("Gamers"));
        m_controls.push_back(&m_leaderboardList);
    }
    {
        m_filterLabel.setupControl(this, currentNode, std::string("Filter"));
        m_controls.push_back(&m_filterLabel);
    }
    {
        m_leaderboardLabel.setupControl(this, currentNode, std::string("Leaderboard"));
        m_controls.push_back(&m_leaderboardLabel);
    }
    {
        m_entriesLabel.setupControl(this, currentNode, std::string("Entries"));
        m_controls.push_back(&m_entriesLabel);
    }
    {
        m_infoLabel.setupControl(this, currentNode, std::string("Info"));
        m_controls.push_back(&m_infoLabel);
    }
    {
        m_tabNavigation.setupControl(this, currentNode, std::string("TabNavigation"));
        m_controls.push_back(&m_tabNavigation);
    }
    return 1;
}

int UIScene_LeaderboardsMenu::ExitLeaderboards(void* context, int pad, C4JStorage::EMessageResult result)
{
    static_cast<UIScene*>(context)->navigateBack();
    return 0;
}

void UIScene_LeaderboardsMenu::updateTooltips()
{
    gConsoleUIController.SetTooltips(static_cast<unsigned int>(m_playerId), -1, IDS_TOOLTIPS_BACK,
                                     IDS_TOOLTIPS_CHANGE_FILTER, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0,
                                     0);
}

void UIScene_LeaderboardsMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, !gApp.GetGameStarted());
    m_layer->showComponent(m_playerId, EUIScene_Logo, false);
}

void UIScene_LeaderboardsMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        if (!gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
        {
            int messageParameters[1];
            messageParameters[0] = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(
                IDS_CONNECTION_LOST, CGameNetworkManager::sInstance.CorrectErrorIDS(IDS_CONNECTION_LOST_LIVE_NO_EXIT),
                messageParameters, 1, gProfile->GetPrimaryPad(), ExitLeaderboards, this, 0, true);
        }
    }
}

std::wstring UIScene_LeaderboardsMenu::getMoviePath()
{
    return std::wstring(L"LeaderboardMenu");
}

int UIScene_LeaderboardsMenu::SetLeaderboardTitleIcons()
{
    int iconCount = 0;
    for (int column = 0; column < 7; ++column)
    {
        int icon = gLeaderboardIconIds[m_statsType * 7 + column];
        if (icon != 0)
        {
            ++iconCount;
            m_leaderboardList.setColumnIcon(column, icon);
        }
    }
    return iconCount;
}

void UIScene_LeaderboardsMenu::PopulateLeaderboard(LeaderboardManager::eStatsReturn result)
{
    SetLeaderboardTitleIcons();
    if (result == LeaderboardManager::eStatsReturn_Success && m_visibleRowCount != 0)
    {
        m_leaderboardList.setupTitles(L10N::GetString(IDS_LEADERBOARD_RANK), L10N::GetString(IDS_LEADERBOARD_GAMERTAG));
        if (gApp.DebugSettingsOn()
            && (GameSettings::GetGameSettingsDebugMask(-1, false) & DEBUG_MASK_FAKE_LEADERBOARD) != 0)
        {
            wchar_t buf[40];
            swprintf(buf, 40, L"%ls12345678", L10N::GetString(IDS_LEADERBOARD_ENTRIES));
            m_entriesLabel.setLabel(UIString(buf), false, false);
            m_infoLabel.setLabel(UIString(L""), false, false);
            m_infoLabel.setVisible(false);
        }
        else
        {
            wchar_t buf[40];
            swprintf(buf, 40, L"%ls%i", L10N::GetString(IDS_LEADERBOARD_ENTRIES), m_visibleRowCount);
            m_entriesLabel.setLabel(UIString(buf), false, false);
            m_infoLabel.setLabel(UIString(L""), false, false);
            m_infoLabel.setVisible(false);
        }

        m_leaderboardList.initLeaderboard(
            m_selectedEntry, static_cast<int>(m_visibleRowCount),
            static_cast<int>(gLeaderboardDescriptors[m_statsType][m_leaderboardIndex].numStats));

        int firstRow = m_startRank;
        int lastRow = firstRow + m_loadedRowCount;
        for (int i = firstRow; i < lastRow; ++i)
        {
            const LeaderboardEntry& entry = m_entries[i];
            bool isLast = (i == lastRow - 1);
            if (entry.messageStringId >= 1)
            {
                m_leaderboardList.addDataSet(isLast, static_cast<int>(entry.rank), static_cast<int>(entry.score),
                                             entry.gamertag, true, L10N::GetString(entry.messageStringId), L"", L"",
                                             L"", L"", L"", L"");
            }
            else
            {
                m_leaderboardList.addDataSet(isLast, static_cast<int>(entry.rank), static_cast<int>(entry.score),
                                             entry.gamertag, false, entry.statText[0], entry.statText[1],
                                             entry.statText[2], entry.statText[3], entry.statText[4], entry.statText[5],
                                             entry.statText[6]);
            }
        }
        m_leaderboardPopulated = true;
        return;
    }

    m_leaderboardList.setupTitles(L"", L"");
    wchar_t buf[40];
    swprintf(buf, 40, L"%ls0", L10N::GetString(IDS_LEADERBOARD_ENTRIES));
    m_entriesLabel.setLabel(UIString(buf), false, false);
    if (result == LeaderboardManager::eStatsReturn_NoScores)
    {
        m_infoLabel.setLabel(UIString(L10N::GetString(IDS_ERROR_NETWORK)), false, false);
        m_infoLabel.setVisible(true);
    }
    else
    {
        m_infoLabel.setLabel(UIString(L10N::GetString(IDS_LEADERBOARD_NORESULTS)), false, false);
        m_infoLabel.setVisible(true);
    }
    m_leaderboardPopulated = true;
}

void UIScene_LeaderboardsMenu::finishUpdatingLeaderboards()
{
    PopulateLeaderboard(static_cast<LeaderboardManager::eStatsReturn>(m_result));
    updateTooltips();
}

void UIScene_LeaderboardsMenu::tick()
{
    UIScene::tick();
    m_leaderboardInterface.tick();
    if (m_result != static_cast<unsigned int>(-1) && gApp.DLCInstallPending() == 0)
    {
        finishUpdatingLeaderboards();
        m_result = static_cast<unsigned int>(-1);
        m_loading = false;
    }
}

void UIScene_LeaderboardsMenu::handleReload()
{
    navigateBack();
}

void UIScene_LeaderboardsMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released,
                                           bool& handled)
{
    if (m_loading && action != EControllerActions_MenuCancel)
        return;

    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    if (!pressed)
        return;

    switch (action)
    {
    case EControllerActions_MenuA:
    case EControllerActions_MenuY:
        handled = true;
        break;
    case EControllerActions_MenuB:
        break;
    case EControllerActions_MenuX:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            if (m_readType == LeaderboardManager::eReadType_Friends)
            {
                m_readType = LeaderboardManager::eReadType_MyScore;
                wchar_t buf[40];
                swprintf(buf, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
                         L10N::GetString(IDS_LEADERBOARD_FILTER_MYSCORE));
                m_filterLabel.setLabel(UIString(buf), false, false);
            }
            else if (m_readType == LeaderboardManager::eReadType_MyScore)
            {
                m_readType = LeaderboardManager::eReadType_TopRank;
                wchar_t buf[40];
                swprintf(buf, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
                         L10N::GetString(IDS_LEADERBOARD_FILTER_OVERALL));
                m_filterLabel.setLabel(UIString(buf), false, false);
            }
            else if (m_readType == LeaderboardManager::eReadType_TopRank)
            {
                m_readType = LeaderboardManager::eReadType_Friends;
                wchar_t buf[40];
                swprintf(buf, 40, L"%ls%ls", L10N::GetString(IDS_LEADERBOARD_FILTER),
                         L10N::GetString(IDS_LEADERBOARD_FILTER_FRIENDS));
                m_filterLabel.setLabel(UIString(buf), false, false);
            }
            ReadStats(-1);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        handled = true;
        break;
    case EControllerActions_MenuUp:
        if (m_selectedEntry > 0)
        {
            --m_selectedEntry;
            sendInputToMovie(action, repeat, pressed, released);
            return;
        }
        m_selectedEntry = 0;
        sendInputToMovie(action, repeat, pressed, released);
        return;
    case EControllerActions_MenuDown:
        if (static_cast<unsigned int>(m_selectedEntry + 1) < m_visibleRowCount)
        {
            ++m_selectedEntry;
        }
        else
        {
            m_selectedEntry = static_cast<int>(m_visibleRowCount) - 1;
        }
        sendInputToMovie(action, repeat, pressed, released);
        return;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuLeft:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            m_statsIconsValid = false;
            int newStatsType = m_statsType;
            if (action == EControllerActions_MenuRight)
            {
                ++newStatsType;
                if (newStatsType == 4)
                    newStatsType = 0;
            }
            else
            {
                if (newStatsType == 0)
                    newStatsType = 4;
                --newStatsType;
            }
            m_statsType = newStatsType;
            unsigned int index = m_leaderboardIndex;
            if (index >= gLeaderboardDescriptors[m_statsType].size())
                index = 0;
            if (m_statsType == 3 && index == 0)
                index = 1;
            m_leaderboardIndex = index;
            SetLeaderboardHeader();
            ReadStats(-1);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            handled = true;
            return;
        }
        handled = true;
        break;
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            if (m_visibleRowCount < 0xb)
                return;
            sendInputToMovie(action, repeat, pressed, released);
            handled = true;
            return;
        }
        handled = true;
        break;
    case EControllerActions_MenuRightScroll:
    case EControllerActions_MenuLeftScroll:
        if (m_leaderboardPopulated && g_pLeaderboardManager->isIdle())
        {
            unsigned int previousIndex = m_leaderboardIndex;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
            unsigned int listSize = static_cast<unsigned int>(gLeaderboardDescriptors[m_statsType].size());
            unsigned int newIndex = m_leaderboardIndex;
            if (action == EControllerActions_MenuRightScroll)
            {
                ++newIndex;
                if (newIndex == listSize)
                    newIndex = 0;
                if (m_statsType == 3 && newIndex == 0)
                    newIndex = 1;
            }
            else
            {
                if (newIndex == 0)
                    newIndex = listSize;
                --newIndex;
                if (m_statsType == 3 && newIndex == 0)
                    newIndex = 3;
            }
            m_leaderboardIndex = newIndex;
            if (previousIndex != newIndex)
            {
                m_tabNavigation.setVisible(true);
                SetLeaderboardHeader();
                ReadStats(-1);
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                handled = true;
                return;
            }
            m_tabNavigation.setVisible(false);
            handled = true;
            return;
        }
        handled = true;
        break;
    case EControllerActions_MenuCancel:
        navigateBack();
        handled = true;
        return;
    default:
        break;
    }
}

void UIScene_LeaderboardsMenu::customDraw(const char* name, fuiRect* rect)
{
    int slot = -1;
    std::sscanf(name, "slot_%d", &slot);
    if (slot == -1)
        return;

    not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(gLeaderboardIconIds[m_statsType * 7 + slot]), 1, 0));
    customDrawSlotControlFui(rect, m_playerId, item, 1.0f, false, false, false);
}

void UIScene_LeaderboardsMenu::handleSelectionChanged(int selection)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    m_selectedEntry = selection;
    updateTooltips();
}

void UIScene_LeaderboardsMenu::handleRequestMoreData(int id, bool forward)
{
    if (m_visibleRowCount == 0)
        return;

    unsigned int index = id + 1;
    unsigned int start = GetEntryStartIndex();
    if (index < start)
    {
        if (g_pLeaderboardManager->isIdle())
        {
            int newStart = static_cast<int>(GetEntryStartIndex()) - 14;
            if (newStart < 1)
                newStart = 1;
            ReadStats(newStart);
        }
    }
    else if (index >= start + m_entries.size() && g_pLeaderboardManager->isIdle())
    {
        ReadStats(static_cast<int>(GetEntryStartIndex() + m_entries.size() + 1));
    }
}

int UIScene_LeaderboardsMenu::getSceneType()
{
    return EUIScene_LeaderboardsMenu;
}

bool UIScene_LeaderboardsMenu::hasFocus(int sceneType)
{
    return m_hasFocus;
}
