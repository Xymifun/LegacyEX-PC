#ifndef GAME_CORE_ISCOREBOARDLAYOUT_HPP
#define GAME_CORE_ISCOREBOARDLAYOUT_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

#include "game/ui/UIString.hpp"

class INetworkPlayer;
class ItemInstance;
class StatsUID;

enum EScoreboardType
{
    EScoreboardType_Scoreboard = 0,
    EScoreboardType_Award = 1
};

struct PlayerInfo
{
    unsigned char smallId;
    signed char voiceStatus;
    short colour;
    std::wstring name;
};

class IScoreboardLayout
{
public:
    IScoreboardLayout();
    virtual ~IScoreboardLayout();

    virtual void Init(int userIndex) = 0;
    virtual bool showLifeSpanProgress() = 0;
    virtual UIString getLabel2(const StatsUID&) = 0;
    virtual UIString getLabel3(const StatsUID&) = 0;
    virtual UIString getLabel4(const StatsUID&) = 0;
    virtual UIString getLabel5(const StatsUID&) = 0;
    virtual UIString getLabel6(const StatsUID&) = 0;
    virtual UIString getLabel7(const StatsUID&) = 0;
    virtual unsigned int getStatIcon2(const EScoreboardType&) = 0;
    virtual unsigned int getStatIcon3(const EScoreboardType&) = 0;
    virtual unsigned int getStatIcon4(const EScoreboardType&) = 0;
    virtual unsigned int getStatIcon5(const EScoreboardType&) = 0;

    virtual unsigned int getTitleIcon(int, const EScoreboardType&);
    virtual std::wstring GetRowLabel(int, const StatsUID&, const EScoreboardType&);
    virtual unsigned int getNumberOfColumns();
    virtual not_null_ptr<ItemInstance> getItemForSlot(int, int, const unsigned int&, const EScoreboardType&) = 0;
    virtual not_null_ptr<ItemInstance> getItemForSlot(const wchar_t*, const unsigned int&, const EScoreboardType&);
    virtual unsigned char GetEndBarIcon(PlayerInfo*) = 0;
    virtual std::wstring GetCustomProgressDetails(const StatsUID&, int&);
    virtual int GetPlayerCount();
    virtual PlayerInfo* GetPlayer(int);
    virtual bool RemovePlayer(int);
    virtual void BuildPlayerInfo(INetworkPlayer*, PlayerInfo*);
    virtual PlayerInfo* CreatePlayerInfo() = 0;
    virtual void AddPlayer(unsigned char) = 0;
    virtual void UpdatePlayer(int, PlayerInfo*, PlayerInfo*, bool) = 0;

    void ClearPlayersInfo();
    static const wchar_t* getIcon(int);
    bool doesExist(const unsigned int&);
    std::wstring getNameOfPlayerAt(const unsigned int&);
    unsigned char getSmallIDOfPlayerAt(const unsigned int&);
    int getVoiceStatusOfPlayerAt(const unsigned int&);
    int getColorStateOfPlayerAt(const unsigned int&);
    PlayerInfo* getPlayersAt(const unsigned int&);

protected:
    int m_userIndex;
    std::vector<PlayerInfo*> m_players;
};

#endif
