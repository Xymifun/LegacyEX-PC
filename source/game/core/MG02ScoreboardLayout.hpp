#ifndef GAME_CORE_MG02SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG02SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"
#include "game/ui/UIString.hpp"

struct MG02PlayerInfo : public PlayerInfo
{
    int totalLifeSpan;
    unsigned char priority;
    unsigned char survived;
    unsigned int roundsWon;
    unsigned int totalBlocksDestroyed;
    unsigned int totalDistanceTravelled;
    int totalTrapsSetOff;
    int totalJumps;
    int deaths;
};

class MG02ScoreboardLayout : public IScoreboardLayout
{
public:
    MG02ScoreboardLayout();
    virtual ~MG02ScoreboardLayout();
    static bool SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int type);
    virtual unsigned int getNumberOfColumns();
    virtual UIString getLabel2(const StatsUID& uid);
    virtual UIString getLabel3(const StatsUID& uid);
    virtual UIString getLabel4(const StatsUID& uid);
    virtual UIString getLabel5(const StatsUID& uid);
    virtual UIString getLabel6(const StatsUID& uid);
    virtual UIString getLabel7(const StatsUID& uid);
    virtual unsigned int getStatIcon2(const EScoreboardType& type);
    virtual unsigned int getStatIcon3(const EScoreboardType& type);
    virtual unsigned int getStatIcon4(const EScoreboardType& type);
    virtual unsigned int getStatIcon5(const EScoreboardType& type);
    virtual unsigned int getTitleIcon(int column, const EScoreboardType& type);
    virtual std::wstring GetRowLabel(int row, const StatsUID& uid, const EScoreboardType& type);
    virtual not_null_ptr<ItemInstance> getItemForSlot(int column, int row, const unsigned int& value,
                                                      const EScoreboardType& type);
    virtual unsigned char GetEndBarIcon(PlayerInfo* info);
    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual void AddPlayer(unsigned char id);
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool changed);
    virtual bool showLifeSpanProgress();
};

#endif
