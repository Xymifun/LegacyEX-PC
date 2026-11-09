#include "game/core/MG02ScoreboardLayout.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG02ScoreboardLayout::MG02ScoreboardLayout() : IScoreboardLayout()
{
}

MG02ScoreboardLayout::~MG02ScoreboardLayout()
{
}

namespace
{
}

bool MG02ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const MG02PlayerInfo* firstInfo = static_cast<const MG02PlayerInfo*>(first);
    const MG02PlayerInfo* secondInfo = static_cast<const MG02PlayerInfo*>(second);
    if (firstInfo->priority)
    {
        if (!secondInfo->priority)
        {
            return true;
        }
    }
    else if (secondInfo->priority)
    {
        return false;
    }

    if (firstInfo->deaths < secondInfo->deaths)
    {
        return true;
    }
    if (firstInfo->deaths > secondInfo->deaths)
    {
        return false;
    }

    if (firstInfo->roundsWon > secondInfo->roundsWon)
    {
        return true;
    }
    if (firstInfo->roundsWon < secondInfo->roundsWon)
    {
        return false;
    }

    if (firstInfo->totalBlocksDestroyed > secondInfo->totalBlocksDestroyed)
    {
        return true;
    }
    if (firstInfo->totalBlocksDestroyed < secondInfo->totalBlocksDestroyed)
    {
        return false;
    }

    if (firstInfo->totalDistanceTravelled < secondInfo->totalDistanceTravelled)
    {
        return true;
    }
    if (firstInfo->totalDistanceTravelled > secondInfo->totalDistanceTravelled)
    {
        return false;
    }

    if (firstInfo->totalLifeSpan > secondInfo->totalLifeSpan)
    {
        return true;
    }
    return false;
}

void MG02ScoreboardLayout::Init(int value)
{
    m_userIndex = value;
    ClearPlayersInfo();

    const unsigned int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[value];

    for (unsigned int index = 0; index < playerCount; ++index)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerByIndex(index);
        if (networkPlayer != 0)
        {
            MG02PlayerInfo* info = new MG02PlayerInfo();
            BuildPlayerInfo(networkPlayer, info);
            m_players.push_back(info);
        }
    }

    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

unsigned int MG02ScoreboardLayout::getNumberOfColumns()
{
    if (Minecraft::GetInstance()->GetMiniGame()->GetLivesPerRound() == 1)
    {
        return 2;
    }
    return 3;
}

UIString MG02ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetTotalBlocksDestroyed(uid);
    std::wstring text(L10N::GetString(IDS_MG_BLOCKS_DESTROYED));
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG02ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int jumps = scoreboard->GetTotalJumps(uid);
    std::wstring text(L10N::GetString(IDS_MG_JUMPS));
    text = replaceAll(text, L"{*JUMPS*}", _toString<unsigned int>(jumps));
    return UIString(text);
}

UIString MG02ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int distance = scoreboard->GetTotalDistanceTravelled(uid);
    std::wstring text(L10N::GetString(IDS_MG_DISTANCE_TRAVELLED));
    text = replaceAll(text, L"{*DELTA*}", _toString<unsigned int>(distance));
    return UIString(text);
}

UIString MG02ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    if (scoreboard->IsOverallWinner(uid))
    {
        return UIString(IDS_MG_WINNER);
    }
    return UIString(L"");
}

UIString MG02ScoreboardLayout::getLabel6(const StatsUID&)
{
    return UIString(L"");
}

UIString MG02ScoreboardLayout::getLabel7(const StatsUID&)
{
    return UIString(0xe063a16f);
}

unsigned int MG02ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    return 5;
}
unsigned int MG02ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    return 6;
}
unsigned int MG02ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    return 7;
}
unsigned int MG02ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 0xffffffff;
}

unsigned int MG02ScoreboardLayout::getTitleIcon(int column, const EScoreboardType& type)
{
    if (column == 0)
        return 4;
    if (column == (int)getNumberOfColumns() - 1 && type != EScoreboardType_Scoreboard)
        return 3;
    return 0xffffffff;
}

std::wstring MG02ScoreboardLayout::GetRowLabel(int row, const StatsUID& uid, const EScoreboardType&)
{
    switch (row)
    {
    case 0:
    {
        ClientScoreboard* scoreboard =
            Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
        return _toString<int>(scoreboard->GetRoundsWon(uid));
    }
    case 1:
        if ((int)getNumberOfColumns() > 2)
        {
            ClientScoreboard* scoreboard =
                Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
            return _toString<int>(scoreboard->GetDeaths(uid, -1));
        }
        break;
    }
    return std::wstring(L"");
}

not_null_ptr<ItemInstance> MG02ScoreboardLayout::getItemForSlot(int column, int, const unsigned int&,
                                                                const EScoreboardType& type)
{
    if (column != -1)
    {
        if (type == EScoreboardType_Scoreboard)
        {
            int auxValue = 3;
            switch (column)
            {
            case 1:
            {
                const int columns = static_cast<int>(getNumberOfColumns());
                if (columns > 2)
                    auxValue = 0;
            }
            case 2:
                return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, auxValue));
            }
        }
        else if (static_cast<int>(getNumberOfColumns()) > 2 && column == 1)
        {
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 0));
        }
    }
    return ItemInstance::EMPTY;
}

unsigned char MG02ScoreboardLayout::GetEndBarIcon(PlayerInfo* baseInfo)
{
    const MG02PlayerInfo* info = static_cast<const MG02PlayerInfo*>(baseInfo);
    unsigned char icon = 0x6e;
    if (info != 0)
    {
        icon = info->priority ^ 1;
    }
    return icon;
}

void MG02ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* baseInfo)
{
    MG02PlayerInfo* info = static_cast<MG02PlayerInfo*>(baseInfo);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    boost::shared_ptr<AbstractClientPlayer> localPlayer = Minecraft::GetInstance()->GetLocalPlayerByName(
        player->GetOnlineName());

    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);

    unsigned char priorityFlag = 0;
    if (localPlayer != 0 && localPlayer->isAlive() && !localPlayer->isSpectator())
    {
        priorityFlag = 1;
    }
    info->priority = priorityFlag;

    info->totalBlocksDestroyed = scoreboard->GetTotalBlocksDestroyed(uid);
    info->survived = scoreboard->GetSurvived(uid, -1);
    info->totalDistanceTravelled = scoreboard->GetTotalDistanceTravelled(uid);
    info->totalLifeSpan = scoreboard->GetTotalLifeSpan(uid);
    info->totalJumps = scoreboard->GetTotalJumps(uid);
    info->totalTrapsSetOff = scoreboard->GetTotalTrapsSetOff(uid);
    info->roundsWon = scoreboard->GetRoundsWon(uid);
    info->deaths = scoreboard->GetDeaths(uid, -1);
}

PlayerInfo* MG02ScoreboardLayout::CreatePlayerInfo()
{
    return new MG02PlayerInfo();
}

void MG02ScoreboardLayout::AddPlayer(unsigned char id)
{
    MG02PlayerInfo* info = new MG02PlayerInfo();
    BuildPlayerInfo(CGameNetworkManager::sInstance.GetPlayerBySmallId(id), info);
    m_players.push_back(info);
}

void MG02ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
    *m_players[index] = *newInfo;
}

bool MG02ScoreboardLayout::showLifeSpanProgress()
{
    return true;
}
