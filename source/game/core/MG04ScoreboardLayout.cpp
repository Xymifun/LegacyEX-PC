#include "game/core/MG04ScoreboardLayout.hpp"
#include "game/core/StringIds.hpp"

#include <algorithm>
#include <cstring>
#include <sstream>

#include "game/core/ClientScoreboard.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

MG04ScoreboardLayout::MG04ScoreboardLayout() : IScoreboardLayout()
{
}

MG04ScoreboardLayout::~MG04ScoreboardLayout()
{
}

namespace
{
}

bool MG04ScoreboardLayout::SORTBYSCORE(PlayerInfo* first, PlayerInfo* second)
{
    const MG04PlayerInfo* firstInfo = static_cast<const MG04PlayerInfo*>(first);
    const MG04PlayerInfo* secondInfo = static_cast<const MG04PlayerInfo*>(second);
    int firstScore = firstInfo->votesReceived[0] + firstInfo->votesReceived[2] + firstInfo->votesReceived[3] * 2;
    int secondScore = secondInfo->votesReceived[0] + secondInfo->votesReceived[2] + secondInfo->votesReceived[3] * 2;
    if (firstScore != secondScore)
    {
        return firstScore > secondScore;
    }
    if (firstInfo->votesReceived[3] != secondInfo->votesReceived[3])
    {
        return firstInfo->votesReceived[3] > secondInfo->votesReceived[3];
    }
    if (firstInfo->votesReceived[2] != secondInfo->votesReceived[2])
    {
        return firstInfo->votesReceived[2] > secondInfo->votesReceived[2];
    }
    int firstValue = firstInfo->votesGiven[0] + firstInfo->votesGiven[2] + firstInfo->votesGiven[3] * 2;
    int secondValue = secondInfo->votesGiven[0] + secondInfo->votesGiven[2] + secondInfo->votesGiven[3] * 2;
    if (firstValue != secondValue)
    {
        return firstValue > secondValue;
    }
    if (firstInfo->votesGiven[3] != secondInfo->votesGiven[3])
    {
        return firstInfo->votesGiven[3] > secondInfo->votesGiven[3];
    }
    if (firstInfo->votesGiven[2] == secondInfo->votesGiven[2])
    {
        return false;
    }
    return firstInfo->votesGiven[2] > secondInfo->votesGiven[2];
}

void MG04ScoreboardLayout::Init(int value)
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
            MG04PlayerInfo* info = new MG04PlayerInfo();
            BuildPlayerInfo(networkPlayer, info);
            m_players.push_back(info);
        }
    }

    std::sort(m_players.begin(), m_players.end(), SORTBYSCORE);
}

UIString MG04ScoreboardLayout::getLabel2(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int blocks = scoreboard->GetBlocksPlaced(uid, -1);
    std::wstring text;
    text = L"";
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel3(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    int round = -1;
    const int blocks = scoreboard->GetBlocksDestroyed(uid, round);
    std::wstring text(L10N::GetString(IDS_MG_BLOCKS_DESTROYED));
    text = replaceAll(text, L"{*BLOCKS*}", _toString<unsigned int>(blocks));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel4(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    const int distance = scoreboard->GetTotalDistanceTravelled(uid);
    const wchar_t* format = L10N::GetString(IDS_MG_DISTANCE_TRAVELLED);
    std::wstring text(format);
    text = replaceAll(text, L"{*DELTA*}", _toString<unsigned int>(distance));
    return UIString(text);
}

UIString MG04ScoreboardLayout::getLabel5(const StatsUID& uid)
{
    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
    if (scoreboard->IsOverallWinner(uid))
    {
        return UIString(IDS_MG_WINNER);
    }
    return UIString(L"");
}

UIString MG04ScoreboardLayout::getLabel6(const StatsUID&)
{
    return UIString(L"");
}

UIString MG04ScoreboardLayout::getLabel7(const StatsUID&)
{
    return UIString(0xe063a16f);
}

unsigned int MG04ScoreboardLayout::getStatIcon2(const EScoreboardType&)
{
    return 5;
}

unsigned int MG04ScoreboardLayout::getStatIcon3(const EScoreboardType&)
{
    return 5;
}

unsigned int MG04ScoreboardLayout::getStatIcon4(const EScoreboardType&)
{
    return 7;
}

unsigned int MG04ScoreboardLayout::getStatIcon5(const EScoreboardType&)
{
    return 7;
}

unsigned int MG04ScoreboardLayout::getTitleIcon(int column, const EScoreboardType&)
{
    switch (column)
    {
    case 0:
    case 1:
    case 2:
        return 0x77;
    default:
        return 0xffffffffu;
    }
}

std::wstring MG04ScoreboardLayout::GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type)
{
    if (type != EScoreboardType_Scoreboard)
    {
        ClientScoreboard* scoreboard =
            Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();
        switch (column)
        {
        case 0:
            return _toString<unsigned short>(scoreboard->GetVotesReceived(uid, -1)[1]);
        case 1:
            return _toString<unsigned short>(scoreboard->GetVotesReceived(uid, -1)[2]);
        case 2:
            return _toString<unsigned short>(scoreboard->GetVotesReceived(uid, -1)[3]);
        }
    }
    return std::wstring(L"");
}

not_null_ptr<ItemInstance> MG04ScoreboardLayout::getItemForSlot(int row, int, const unsigned int&,
                                                                const EScoreboardType& type)
{
    if (type == EScoreboardType_Scoreboard)
    {
        if (row == 3)
        {
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
        }
    }
    else
    {
        switch (row)
        {
        case 0:
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::ironIngot));
        case 1:
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::goldIngot));
        case 2:
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::diamond));
        case 3:
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::skull, 1, 3));
        }
    }
    return ItemInstance::EMPTY;
}

unsigned char MG04ScoreboardLayout::GetEndBarIcon(PlayerInfo* info)
{
    if (info != 0)
    {
        return 0;
    }
    return 0x6e;
}

void MG04ScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* baseInfo)
{
    MG04PlayerInfo* info = static_cast<MG04PlayerInfo*>(baseInfo);
    const StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    IScoreboardLayout::BuildPlayerInfo(player, info);

    boost::shared_ptr<AbstractClientPlayer> localPlayer = Minecraft::GetInstance()->GetLocalPlayerByName(
        player->GetOnlineName());

    ClientScoreboard* scoreboard =
        Minecraft::GetInstance()->mLocalPlayers[m_userIndex]->GetLevel()->GetClientScoreboard();

    memcpy(info->votesReceived, scoreboard->GetVotesReceived(uid, -1), sizeof(info->votesReceived));
    memcpy(info->votesGiven, scoreboard->GetVotesGiven(uid, -1), sizeof(info->votesGiven));
    info->blocksPlaced = scoreboard->GetBlocksPlaced(uid, -1);
    info->blocksDestroyed = scoreboard->GetBlocksDestroyed(uid, -1);
}

PlayerInfo* MG04ScoreboardLayout::CreatePlayerInfo()
{
    return new MG04PlayerInfo();
}

void MG04ScoreboardLayout::AddPlayer(unsigned char smallId)
{
    MG04PlayerInfo* info = new MG04PlayerInfo();
    BuildPlayerInfo(CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId), info);
    m_players.push_back(info);
}

void MG04ScoreboardLayout::UpdatePlayer(int index, PlayerInfo*, PlayerInfo* newInfo, bool)
{
    *m_players[index] = *newInfo;
}

bool MG04ScoreboardLayout::showLifeSpanProgress()
{
    return true;
}

unsigned int MG04ScoreboardLayout::getNumberOfColumns()
{
    return 4;
}
