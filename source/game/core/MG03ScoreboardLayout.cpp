#include "game/core/MG03ScoreboardLayout.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <sstream>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/GameModeOptions.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/System.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringConversion.hpp"
#include "game/ui/UIScene_HUD.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG03ScoreboardLayout::MG03ScoreboardLayout(EScoreboardType type) : IScoreboardLayout(), m_type(type)
{
}

MG03ScoreboardLayout::~MG03ScoreboardLayout()
{
}

bool MG03ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const MG03PlayerInfo* firstInfo = static_cast<const MG03PlayerInfo*>(first);
    const MG03PlayerInfo* secondInfo = static_cast<const MG03PlayerInfo*>(second);
    if (firstInfo->highlight == 0)
    {
        if (secondInfo->highlight != 0)
        {
            return true;
        }
    }
    else if (secondInfo->highlight == 0)
    {
        return false;
    }

    if (firstInfo->points > secondInfo->points)
    {
        return true;
    }
    if (firstInfo->points < secondInfo->points)
    {
        return false;
    }

    if (firstInfo->endZoneEntered != 0)
    {
        if (secondInfo->endZoneEntered == 0)
        {
            return true;
        }
    }
    else if (secondInfo->endZoneEntered != 0)
    {
        return false;
    }

    if (firstInfo->lifeSpan < secondInfo->lifeSpan)
    {
        return true;
    }
    if (firstInfo->lifeSpan > secondInfo->lifeSpan)
    {
        return false;
    }

    if (firstInfo->secondaryTargets > secondInfo->secondaryTargets)
    {
        return true;
    }
    if (firstInfo->secondaryTargets < secondInfo->secondaryTargets)
    {
        return false;
    }

    if (firstInfo->mapTraversed > secondInfo->mapTraversed)
    {
        return true;
    }
    return false;
}

void MG03ScoreboardLayout::Init(int value)
{
    m_userIndex = value;
    ClearPlayersInfo();
    const unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[value];
    for (unsigned int index = 0; index < playerCount; ++index)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(index);
        if (player != 0)
        {
            MG03PlayerInfo* info = new MG03PlayerInfo();
            BuildPlayerInfo(player, info);
            m_players.push_back(info);
        }
    }
    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

unsigned int MG03ScoreboardLayout::getNumberOfColumns()
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return miniGame->ArePointsActive() ? 5 : 3;
}

std::wstring MG03ScoreboardLayout::getStatForRound(const StatsUID& uid, int round)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();

    if (miniGame->ArePointsActive())
    {
        return formatwstr(L"%d", scoreboard->GetPoints(uid, round));
    }

    if (scoreboard->GetEndZoneEntered(uid, round))
    {
        return UIScene_HUD::formatTime(scoreboard->GetLifeSpan(uid, round), false);
    }

    return L10N::GetString(IDS_DNF);
}

UIString MG03ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    return UIString(getStatForRound(uid, 0));
}

UIString MG03ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
    {
        return UIString(getStatForRound(uid, 1));
    }
    return UIString(L"");
}

UIString MG03ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
    {
        return UIString(getStatForRound(uid, 2));
    }
    return UIString(L"");
}

UIString MG03ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (!scoreboard->HasStats(uid))
    {
        return UIString(IDS_MG_SPECTATOR);
    }
    if (scoreboard->IsOverallGlideWinner(uid, miniGame->ArePointsActive()))
    {
        return UIString(IDS_MG_WINNER);
    }
    return UIString(L"");
}

UIString MG03ScoreboardLayout::getLabel6(const StatsUID&)
{
    return UIString(L"");
}

UIString MG03ScoreboardLayout::getLabel7(const StatsUID&)
{
    return UIString(L"");
}

unsigned int MG03ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    return miniGame->ArePointsActive() ? 0x70 : 0x74;
}

unsigned int MG03ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    unsigned int icon;
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        icon = 0x75;
        if (miniGame->ArePointsActive())
            return 0x71;
    }
    else
    {
        icon = 0xffffffffu;
    }
    return icon;
}

unsigned int MG03ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    unsigned int icon;
    if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        icon = 0x76;
        if (miniGame->ArePointsActive())
            return 0x72;
    }
    else
    {
        icon = 0xffffffffu;
    }
    return icon;
}

unsigned int MG03ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 0xffffffffu;
}

unsigned int MG03ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    if (miniGame->ArePointsActive())
    {
        switch (column)
        {
        case 0:
            return 0x6f;
        case 2:
            return 0x73;
        case 4:
            return type == EScoreboardType_Scoreboard ? 0x77 : 3;
        }
    }
    else
    {
        switch (column)
        {
        case 0:
            return 0x73;
        case 2:
            return type == EScoreboardType_Scoreboard ? 0x77 : 3;
        }
    }
    return 0xffffffffu;
}

std::wstring MG03ScoreboardLayout::GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();

    if (miniGame->ArePointsActive())
    {
        switch (column)
        {
        case 0:
        {
            int points;
            if (type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
            {
                int round = -1;
                points = scoreboard->GetBestPoints(uid, &round);
                if (!scoreboard->GetEndZoneEntered(uid, round))
                    return L10N::GetString(IDS_DNF);
            }
            else
            {
                points = scoreboard->GetPoints(uid, -1);
            }
            return formatwstr(L"%d", points);
        }
        case 2:
            if (type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
            {
                int round = -1;
                scoreboard->GetBestPoints(uid, &round);
                if (!scoreboard->GetEndZoneEntered(uid, round))
                    return L10N::GetString(IDS_DNF);
                return UIScene_HUD::formatTime(scoreboard->GetLifeSpan(uid, round), false);
            }
            if (scoreboard->GetEndZoneEntered(uid, -1))
                return UIScene_HUD::formatTime(scoreboard->GetLifeSpan(uid, -1), false);
            return L"---";
        }
    }
    else
    {
        if (column == 0)
        {
            if (type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
            {
                int round = -1;
                const long long lifeSpan = scoreboard->GetBestLifeSpan(uid, &round);
                if (!scoreboard->GetEndZoneEntered(uid, round))
                {
                    if ((gameMode->IsWaitingToStart() || gameMode->IsRoundStarting()) && scoreboard->mCurrentRound == 0)
                        return L"---";
                    return L10N::GetString(IDS_DNF);
                }
                return UIScene_HUD::formatTime(lifeSpan, false);
            }
            if (scoreboard->GetEndZoneEntered(uid, -1))
                return UIScene_HUD::formatTime(scoreboard->GetLifeSpan(uid, -1), false);
            return L"---";
        }
    }
    return L"";
}

not_null_ptr<ItemInstance> MG03ScoreboardLayout::getItemForSlot(int row, int column, const unsigned int& value,
                                                                const EScoreboardType& type)
{
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    if (type == EScoreboardType_Scoreboard && row != -1)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        if (miniGame->ArePointsActive())
        {
            if (row == 4)
            {
                item = not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
            }
        }
        else if (row == 2)
        {
            item = not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
        }
    }

    return item;
}

unsigned char MG03ScoreboardLayout::GetEndBarIcon(PlayerInfo* baseInfo)
{
    const MG03PlayerInfo* info = static_cast<const MG03PlayerInfo*>(baseInfo);
    unsigned char icon = 0;
    if (info->teamInvite != 0)
    {
        icon = 2;
    }
    return icon;
}

void MG03ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* baseInfo)
{
    MG03PlayerInfo* info = static_cast<MG03PlayerInfo*>(baseInfo);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    boost::shared_ptr<AbstractClientPlayer> localPlayer = Minecraft::GetInstance()->GetLocalPlayerByName(
        player->GetOnlineName());

    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    ClientMasterGameMode* gameMode = Minecraft::GetInstance()->GetClientMasterGameMode();
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();

    const bool isSpectating = localPlayer != 0 && localPlayer->isSpectator();

    int secondaryTargets = 0;
    int points = 0;
    int lifespan;
    unsigned char endZoneEntered;
    double mapTraversed;

    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);

    if (miniGame->ArePointsActive())
    {
        if (m_type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
        {
            int round = -1;
            points = scoreboard->GetBestPoints(uid, &round);
            lifespan = scoreboard->GetLifeSpan(uid, round);
            secondaryTargets = scoreboard->GetSecondaryTargets(uid, round);
            endZoneEntered = scoreboard->GetEndZoneEntered(uid, round);
            mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, round);
        }
        else
        {
            points = scoreboard->GetPoints(uid, -1);
            if (scoreboard->GetEndZoneEntered(uid, -1))
            {
                lifespan = scoreboard->GetLifeSpan(uid, -1);
                secondaryTargets = scoreboard->GetSecondaryTargets(uid, -1);
                endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
                mapTraversed = 100.0;
            }
            else
            {
                lifespan = static_cast<int>(System::processTimeInMilliSecs() - gameMode->GetLocalRoundTimeStart());
                if (gameMode->IsInLobbyState() || !scoreboard->HasStats(uid))
                {
                    lifespan = 0;
                }
                secondaryTargets = scoreboard->GetSecondaryTargets(uid, -1);
                endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
                mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, -1);
            }
        }
    }
    else
    {
        if (m_type == EScoreboardType_Award || gameMode->IsWaitingToStart() || gameMode->IsRoundStarting())
        {
            int round = -1;
            lifespan = scoreboard->GetBestLifeSpan(uid, &round);
            endZoneEntered = scoreboard->GetEndZoneEntered(uid, round);
            mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, round);
        }
        else
        {
            if (scoreboard->GetEndZoneEntered(uid, -1))
            {
                lifespan = scoreboard->GetLifeSpan(uid, -1);
                endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
                mapTraversed = 100.0;
            }
            else
            {
                lifespan = static_cast<int>(System::processTimeInMilliSecs() - gameMode->GetLocalRoundTimeStart());
                if (gameMode->IsInLobbyState() || !scoreboard->HasStats(uid))
                {
                    lifespan = 0;
                }
                endZoneEntered = scoreboard->GetEndZoneEntered(uid, -1);
                mapTraversed = scoreboard->GetPercentageOfMapTraversed(uid, -1);
            }
        }
    }

    info->highlight = isSpectating && !endZoneEntered;
    info->endZoneEntered = endZoneEntered;
    info->lifeSpan = lifespan;
    info->points = points;
    info->secondaryTargets = secondaryTargets;
    info->mapTraversed = mapTraversed;

    MultiPlayerGameMode* localGameMode = Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLocalGameMode();
    info->teamInvite = static_cast<unsigned char>(localGameMode->IsTeamInvite(player->GetOnlineName()));
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
}

PlayerInfo* MG03ScoreboardLayout::CreatePlayerInfo()
{
    return new MG03PlayerInfo();
}

void MG03ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    MG03PlayerInfo* info = new MG03PlayerInfo();
    BuildPlayerInfo(CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId), info);
    m_players.push_back(info);
}

void MG03ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
    *m_players[index] = *newInfo;
}

std::wstring MG03ScoreboardLayout::GetCustomProgressDetails(const StatsUID& uid, int& value)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    std::wstring details = L"";
    int round = -1;

    if (miniGame->ArePointsActive())
    {
        const unsigned int personal = scoreboard->GetBestPoints(uid, &round);
        const unsigned int best = scoreboard->GetBestPoints();
        const unsigned int behind = best - personal;
        if (behind != 0)
        {
            value = static_cast<int>((static_cast<float>(personal) / static_cast<float>(best)) * 100.0f);
            details = formatwstr(L"-%d", behind);
        }
        else
        {
            value = 100;
            details = formatwstr(L"%d", personal);
        }
    }
    else
    {
        const unsigned int personal = scoreboard->GetBestLifeSpan(uid, &round);
        const unsigned int best = scoreboard->GetBestLifeSpan();
        const unsigned int behind = personal - best;
        if (behind != 0)
        {
            value = static_cast<int>((static_cast<float>(best) / static_cast<float>(personal)) * 100.0f);
            details = formatwstr(L"+%d:%.2d.%.3d", (behind / 60000) % 60, (behind / 1000) % 60, behind % 1000);
        }
        else
        {
            value = 100;
            details = formatwstr(L"%d:%.2d.%.3d", (personal / 60000) % 60, (personal / 1000) % 60, personal % 1000);
        }
    }

    if (!scoreboard->GetEndZoneEntered(uid, round))
    {
        value = 0;
        return L10N::GetString(IDS_DNF);
    }
    return details;
}

bool MG03ScoreboardLayout::showLifeSpanProgress()
{
    return false;
}
