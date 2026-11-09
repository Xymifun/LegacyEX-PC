#ifndef GAME_CORE_MG03SCOREBOARDLAYOUT_HPP
#define GAME_CORE_MG03SCOREBOARDLAYOUT_HPP

#include "game/core/IScoreboardLayout.hpp"
#include "game/ui/UIString.hpp"

struct MG03PlayerInfo : public PlayerInfo
{
    int secondaryTargets;
    int points;
    long long lifeSpan;
    unsigned char highlight;
    unsigned char endZoneEntered;
    unsigned char teamInvite;
    double mapTraversed;
};

class MG03ScoreboardLayout : public IScoreboardLayout
{
private:
    EScoreboardType m_type;

public:
    explicit MG03ScoreboardLayout(EScoreboardType type);
    virtual ~MG03ScoreboardLayout();
    static bool SORTBYSCORE(PlayerInfo* first, PlayerInfo* second);
    void Init(int value);
    virtual unsigned int getStatIcon2(const EScoreboardType& type);
    virtual unsigned int getStatIcon3(const EScoreboardType& type);
    virtual unsigned int getStatIcon4(const EScoreboardType& type);
    virtual unsigned int getStatIcon5(const EScoreboardType& type);
    virtual UIString getLabel2(const StatsUID& uid);
    virtual UIString getLabel3(const StatsUID& uid);
    virtual UIString getLabel4(const StatsUID& uid);
    virtual UIString getLabel5(const StatsUID& uid);
    virtual UIString getLabel6(const StatsUID& uid);
    virtual UIString getLabel7(const StatsUID& uid);
    virtual unsigned int getTitleIcon(int column, const EScoreboardType& type);
    virtual std::wstring GetRowLabel(int row, const StatsUID& uid, const EScoreboardType& type);
    virtual unsigned int getNumberOfColumns();
    not_null_ptr<ItemInstance> getItemForSlot(int row, int column, const unsigned int& value,
                                              const EScoreboardType& type);
    virtual unsigned char GetEndBarIcon(PlayerInfo* info);
    virtual std::wstring GetCustomProgressDetails(const StatsUID& uid, int& value);
    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual void AddPlayer(unsigned char smallId);
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo, bool changed);
    std::wstring getStatForRound(const StatsUID& uid, int round);
    virtual bool showLifeSpanProgress();
};

#endif
