#include "game/core/IScoreboardLayout.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/network/INetworkPlayer.hpp"
#include <cwchar>

static const unsigned int NUM_ICONS = 119;

static const wchar_t* const ICON_NAMES[NUM_ICONS] = {L"Alive",
                                                     L"Dead",
                                                     L"Alliance",
                                                     L"AwardIcon",
                                                     L"VictoryCountIcon",
                                                     L"BlocksDestroyedIcon",
                                                     L"JumpsIcon",
                                                     L"DistanceTravelledIcon",
                                                     L"VictoryCountIcon",
                                                     L"BlocksDestroyedIcon",
                                                     L"JumpsIcon",
                                                     L"DistanceTravelledIcon",
                                                     L"AwardIcons_0000",
                                                     L"AwardIcons_0001",
                                                     L"AwardIcons_0002",
                                                     L"AwardIcons_0003",
                                                     L"AwardIcons_0004",
                                                     L"AwardIcons_0005",
                                                     L"AwardIcons_0006",
                                                     L"AwardIcons_0007",
                                                     L"AwardIcons_0008",
                                                     L"AwardIcons_0009",
                                                     L"AwardIcons_0010",
                                                     L"AwardIcons_0011",
                                                     L"AwardIcons_0012",
                                                     L"AwardIcons_0013",
                                                     L"AwardIcons_0014",
                                                     L"AwardIcons_0015",
                                                     L"AwardIcons_0016",
                                                     L"AwardIcons_0017",
                                                     L"AwardIcons_0018",
                                                     L"AwardIcons_0019",
                                                     L"AwardIcons_0020",
                                                     L"AwardIcons_0021",
                                                     L"AwardIcons_0022",
                                                     L"AwardIcons_0023",
                                                     L"AwardIcons_0024",
                                                     L"AwardIcons_0025",
                                                     L"AwardIcons_0026",
                                                     L"AwardIcons_0027",
                                                     L"AwardIcons_0028",
                                                     L"AwardIcons_0029",
                                                     L"AwardIcons_0030",
                                                     L"AwardIcons_0031",
                                                     L"AwardIcons_0032",
                                                     L"AwardIcons_0033",
                                                     L"AwardIcons_0034",
                                                     L"AwardIcons_0035",
                                                     L"AwardIcons_0036",
                                                     L"AwardIcons_0037",
                                                     L"AwardIcons_0038",
                                                     L"AwardIcons_0039",
                                                     L"AwardIcons_0040",
                                                     L"AwardIcons_0041",
                                                     L"AwardIcons_0042",
                                                     L"AwardIcons_0043",
                                                     L"AwardIcons_0044",
                                                     L"AwardIcons_0045",
                                                     L"AwardIcons_0046",
                                                     L"AwardIcons_0047",
                                                     L"AwardIcons_0048",
                                                     L"AwardIcons_0049",
                                                     L"AwardIcons_0050",
                                                     L"AwardIcons_0051",
                                                     L"AwardIcons_0052",
                                                     L"AwardIcons_0053",
                                                     L"AwardIcons_0054",
                                                     L"AwardIcons_0055",
                                                     L"AwardIcons_0056",
                                                     L"AwardIcons_0057",
                                                     L"AwardIcons_0058",
                                                     L"AwardIcons_0059",
                                                     L"AwardIcons_0060",
                                                     L"AwardIcons_0026",
                                                     L"AwardIcons_0028",
                                                     L"AwardIcons_0024",
                                                     L"AwardIcons_0061",
                                                     L"AwardIcons_0062",
                                                     L"AwardIcons_0063",
                                                     L"AwardIcons_0064",
                                                     L"AwardIcons_0065",
                                                     L"AwardIcons_0066",
                                                     L"AwardIcons_0067",
                                                     L"AwardIcons_0068",
                                                     L"AwardIcons_0069",
                                                     L"AwardIcons_0070",
                                                     L"AwardIcons_0071",
                                                     L"AwardIcons_0072",
                                                     L"AwardIcons_0073",
                                                     L"AwardIcons_0074",
                                                     L"AwardIcons_0075",
                                                     L"AwardIcons_0076",
                                                     L"AwardIcons_0077",
                                                     L"AwardIcons_0078",
                                                     L"AwardIcons_0079",
                                                     L"AwardIcons_0080",
                                                     L"AwardIcons_0081",
                                                     L"AwardIcons_0082",
                                                     L"AwardIcons_0083",
                                                     L"AwardIcons_0084",
                                                     L"AwardIcons_0085",
                                                     L"AwardIcons_0086",
                                                     L"AwardIcons_0087",
                                                     L"AwardIcons_0088",
                                                     L"AwardIcons_0089",
                                                     L"AwardIcons_0090",
                                                     L"AwardIcons_0091",
                                                     L"AwardIcons_0092",
                                                     L"AwardIcons_0093",
                                                     L"AwardIcons_0094",
                                                     L"SpectatorIcon",
                                                     L"ScoreIcon",
                                                     L"Score1Icon",
                                                     L"Score2Icon",
                                                     L"Score3Icon",
                                                     L"TimeIcon",
                                                     L"Time1Icon",
                                                     L"Time2Icon",
                                                     L"Time3Icon"};

IScoreboardLayout::IScoreboardLayout() : m_players()
{
    m_userIndex = 0;
}

IScoreboardLayout::~IScoreboardLayout()
{
    ClearPlayersInfo();
}

void IScoreboardLayout::ClearPlayersInfo()
{
    for (unsigned int index = 0; index < m_players.size(); ++index)
    {
        if (m_players[index] != 0)
        {
            delete m_players[index];
            m_players[index] = 0;
        }
    }
    m_players.clear();
}

not_null_ptr<ItemInstance> IScoreboardLayout::getItemForSlot(const wchar_t* value, const unsigned int& slot,
                                                             const EScoreboardType& type)
{
    int header = -1;
    int stat = -1;
    swscanf(value, L"slot_header_%d", &header);
    swscanf(value, L"slot_stat_%d", &stat);
    return getItemForSlot(header, stat, slot, type);
}

const wchar_t* IScoreboardLayout::getIcon(int index)
{
    const wchar_t* icon = L"";
    if (index < NUM_ICONS)
    {
        icon = ICON_NAMES[index];
    }
    return icon;
}

bool IScoreboardLayout::doesExist(const unsigned int& index)
{
    if (index < m_players.size())
    {
        return m_players[index] != 0;
    }
    return false;
}

std::wstring IScoreboardLayout::getNameOfPlayerAt(const unsigned int& index)
{
    if (index < m_players.size())
    {
        return m_players[index]->name;
    }
    return std::wstring(L"");
}

unsigned char IScoreboardLayout::getSmallIDOfPlayerAt(const unsigned int& index)
{
    if (index < m_players.size())
    {
        return m_players[index]->smallId;
    }
    return 0;
}

int IScoreboardLayout::getVoiceStatusOfPlayerAt(const unsigned int& index)
{
    if (index < m_players.size())
    {
        return m_players[index]->voiceStatus;
    }
    return 0;
}

int IScoreboardLayout::getColorStateOfPlayerAt(const unsigned int& index)
{
    if (index < m_players.size())
    {
        return m_players[index]->colour;
    }
    return 0;
}

int IScoreboardLayout::GetPlayerCount()
{
    return static_cast<int>(m_players.size());
}

PlayerInfo* IScoreboardLayout::GetPlayer(int index)
{
    return m_players[index];
}

bool IScoreboardLayout::RemovePlayer(int index)
{
    bool removable = index >= 0;
    if (removable)
    {
        if (m_players[index] != 0)
        {
            delete m_players[index];
        }
        m_players[index] = 0;
        m_players.erase(m_players.begin() + index);
    }
    return removable;
}

void IScoreboardLayout::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    info->smallId = player->GetSmallId();

    std::wstring displayName(L"");
    displayName = player->GetDisplayName();

    char voiceStatus = 0;
    if (player->HasVoice())
    {
        if (player->IsMutedByLocalUser(m_userIndex))
        {
            voiceStatus = 3;
        }
        else
        {
            voiceStatus = 1;
            if (player->IsTalking())
            {
                voiceStatus = 2;
            }
        }
    }

    info->voiceStatus = voiceStatus;
    info->colour = static_cast<short>(gApp.GetPlayerColour(info->smallId));
    info->name = displayName;
}

unsigned int IScoreboardLayout::getTitleIcon(int, const EScoreboardType&)
{
    return 0xffffffffu;
}

std::wstring IScoreboardLayout::GetRowLabel(int, const StatsUID&, const EScoreboardType&)
{
    return std::wstring(L"");
}

unsigned int IScoreboardLayout::getNumberOfColumns()
{
    return 2;
}

std::wstring IScoreboardLayout::GetCustomProgressDetails(const StatsUID&, int&)
{
    return std::wstring(L"");
}
