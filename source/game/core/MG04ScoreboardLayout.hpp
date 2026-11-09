#ifndef GAME_CORE_MG04SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG04SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"
#include "game/ui/UIString.hpp"

struct MG04PlayerInfo : public PlayerInfo
{
    int votesGiven[4];
    int votesReceived[4];
    int blocksPlaced;
    int blocksDestroyed;
};

class MG04ScoreboardLayout : public IScoreboardLayout
{
public:
    MG04ScoreboardLayout();
    virtual ~MG04ScoreboardLayout();
    static bool SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int value);
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
    virtual not_null_ptr<ItemInstance> getItemForSlot(int row, int column, const unsigned int& slot,
                                                      const EScoreboardType& type);
    virtual unsigned char GetEndBarIcon(PlayerInfo* info);
    void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    PlayerInfo* CreatePlayerInfo();
    void AddPlayer(unsigned char smallId);
    void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool changed);
    virtual bool showLifeSpanProgress();
    unsigned int getNumberOfColumns();
};

#endif
