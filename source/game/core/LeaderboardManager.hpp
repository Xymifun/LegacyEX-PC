#ifndef GAME_CORE_LEADERBOARDMANAGER_HPP
#define GAME_CORE_LEADERBOARDMANAGER_HPP

#include <string>

#include "game/world/generation/PlayerUID.hpp"

class LeaderboardReadListener;

class LeaderboardManager
{
public:
    enum EStatsType
    {
        eStatsType_Score,
        eStatsType_BlocksBroken,
        eStatsType_BlocksPlaced,
        eStatsType_MobsKilled,
        eStatsType_DistanceTravelled,
        eStatsType_TimePlayed,
        eStatsType_ItemsCrafted,
        eStatsType_Count
    };

    enum eStatsReturn
    {
        eStatsReturn_Success,
        eStatsReturn_Failed,
        eStatsReturn_NoScores
    };

    enum EReadType
    {
        eReadType_Friends,
        eReadType_MyScore,
        eReadType_TopRank,
        eReadType_Write,
        eReadType_None
    };

    enum
    {
        MAX_STATS_PER_SCORE = 8
    };

    class ReadScore
    {
    public:
        PlayerUID m_uid;
        unsigned int m_uiRank;
        std::wstring m_wsGamertag;
        unsigned int m_uiRating;
        unsigned short m_usNumStats;
        unsigned int m_auiStats[MAX_STATS_PER_SCORE];
        int m_iMessageStringId;
    };

    class RegisterScore
    {
    public:
        EStatsType m_eStatsType;
        unsigned short m_usNumStats;
        unsigned int m_auiStats[MAX_STATS_PER_SCORE];
    };

    class ReadView
    {
    public:
        unsigned int m_uiNumScores;
        ReadScore* m_pScores;
    };

    LeaderboardManager();

    virtual ~LeaderboardManager() {}
    virtual void zeroReadParameters();
    virtual void Tick() = 0;
    virtual bool OpenSession() = 0;
    virtual void CloseSession() = 0;
    virtual void DeleteSession() = 0;
    virtual bool WriteStats(unsigned int uiNumScores, RegisterScore* pScores) = 0;
    virtual bool ReadStats_Friends(LeaderboardReadListener* pReadListener, int iUserIndex, EStatsType eStatsType,
                                   PlayerUID uid, unsigned int uiStartRank, unsigned int uiNumRows);
    virtual bool ReadStats_MyScore(LeaderboardReadListener* pReadListener, int iUserIndex, EStatsType eStatsType,
                                   PlayerUID uid, unsigned int uiNumRows);
    virtual bool ReadStats_TopRank(LeaderboardReadListener* pReadListener, int iUserIndex, EStatsType eStatsType,
                                   unsigned int uiStartRank, unsigned int uiNumRows);
    virtual void FlushStats() = 0;
    virtual void CancelOperation() = 0;
    virtual bool isIdle() = 0;

    static void DeleteInstance();
    static void printStats(ReadView& readView);

protected:
    EReadType m_eReadType;
    int m_iUserIndex;
    EStatsType m_eStatsType;
    LeaderboardReadListener* m_pReadListener;
    PlayerUID m_uid;
    unsigned int m_uiStartRank;
    unsigned int m_uiNumRows;
};

extern LeaderboardManager* g_pLeaderboardManager;

#endif
