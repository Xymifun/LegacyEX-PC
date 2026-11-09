#ifndef GAME_CORE_MG01SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG01SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"
#include "game/ui/UIString.hpp"

struct MG01PlayerInfo : public PlayerInfo
{
    int roundsWon;
    int kills;
    int lifeSpan;
    int damageDealt;
    int damageReceived;
    unsigned char localPlayer;
    unsigned char survived;
    unsigned char teamInvite;
};

class MG01ScoreboardLayout : public IScoreboardLayout
{
private:
    EScoreboardType m_type;

public:
    explicit MG01ScoreboardLayout(EScoreboardType type);
    virtual ~MG01ScoreboardLayout();
    static bool SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int value);
    unsigned int getNumberOfColumns();
    not_null_ptr<ItemInstance> getItemForSlot(int row, int column, const unsigned int& slot,
                                              const EScoreboardType& type);
    UIString getLabel2(const StatsUID& uid);
    UIString getLabel3(const StatsUID& uid);
    UIString getLabel4(const StatsUID& uid);
    UIString getLabel5(const StatsUID& uid);
    UIString getLabel6(const StatsUID& uid);
    UIString getLabel7(const StatsUID& uid);
    unsigned int getStatIcon2(const EScoreboardType& type);
    unsigned int getStatIcon3(const EScoreboardType& type);
    unsigned int getStatIcon4(const EScoreboardType& type);
    unsigned int getStatIcon5(const EScoreboardType& type);
    unsigned int getTitleIcon(int column, const EScoreboardType& type);
    std::wstring GetRowLabel(int column, const StatsUID& uid, const EScoreboardType& type);
    void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    PlayerInfo* CreatePlayerInfo();
    void AddPlayer(unsigned char smallId);
    void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool flag);
    unsigned char GetEndBarIcon(PlayerInfo* info);
    bool showLifeSpanProgress();
};

#endif
