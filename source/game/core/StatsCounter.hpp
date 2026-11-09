#ifndef GAME_CORE_STATSCOUNTER_HPP
#define GAME_CORE_STATSCOUNTER_HPP

#include <boost/unordered_map.hpp>

class Achievement;
class Stat;

class StatsCounter : public boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >
{
public:
    enum ESubTypes
    {
        eSubtype0 = 0,
        eSubtype1 = 1,
        eSubtype2 = 2,
        eSubtype3 = 3
    };

    enum LEADERBOARD_FLAG
    {
        eLeaderboard0 = 0,
        eLeaderboard1 = 1,
        eLeaderboard2 = 2,
        eLeaderboard3 = 3,
        eLeaderboard4 = 4,
        eLeaderboard5 = 5,
        eLeaderboard6 = 6,
        eLeaderboard7 = 7,
        eLeaderboard8 = 8,
        eLeaderboard9 = 9,
        eLeaderboard10 = 10,
        eLeaderboard11 = 11,
        eLeaderboard12 = 12,
        eLeaderboard13 = 13,
        eLeaderboard14 = 14,
        eLeaderboard15 = 15,
        eLeaderboard16 = 16,
        eLeaderboard17 = 17,
        eLeaderboard18 = 18,
        eLeaderboard19 = 19,
        eLeaderboard20 = 20,
        eLeaderboard21 = 21,
        eLeaderboard22 = 22,
        eLeaderboard23 = 23,
        eLeaderboard24 = 24,
        eLeaderboard25 = 25,
        eLeaderboard26 = 26,
        eLeaderboard27 = 27,
        eLeaderboard28 = 28,
        eLeaderboard29 = 29,
        eLeaderboard30 = 30,
        eLeaderboard31 = 31,
        eLeaderboard32 = 32,
        eLeaderboard33 = 33,
        eLeaderboard34 = 34,
        eLeaderboard35 = 35,
        eLeaderboard36 = 36,
        eLeaderboard37 = 37,
        eLeaderboard38 = 38,
        eLeaderboard39 = 39,
        eLeaderboard40 = 40,
        eLeaderboard41 = 41,
        eLeaderboard42 = 42,
        eLeaderboard43 = 43,
        eLeaderboard44 = 44,
        eLeaderboard45 = 45,
        eLeaderboard46 = 46,
        eLeaderboard47 = 47,
        eLeaderboard48 = 48
    };

    static boost::unordered_map<Stat*, boost::unordered_map<unsigned int, LEADERBOARD_FLAG> > statBoards;

    StatsCounter();
    static void setupStatBoards();
    unsigned int isLargeStat(Stat* stat);
    void award(Stat* stat, ESubTypes subtype, unsigned int amount, bool accumulate);
    int getTotalValue(Stat* stat);
    void save(int profile, bool force, unsigned char* data);
    void writeStats();
    void flushLeaderboards();
    void tick(int profile);
    void parse(void* data);
    void parseOld(unsigned short version, void* data);
    void clear();
    void saveLeaderboards();
    bool hasTaken(Achievement* achievement);
    bool canTake(Achievement* achievement);
    unsigned int getValue(Stat* stat, unsigned int subtype);
    void dumpStatsToTTY();

private:
    static const unsigned int PROFILE_STATS_OFFSET = 204;
    static const unsigned int PROFILE_STATS_SIZE = 1740;
    static const int SAVE_DELAY = 1800;

    unsigned char mSavePending;
    int mSaveDelay;
    boost::unordered_map<unsigned int, bool> mLeaderboardPending;
    int mLeaderboardDelay;
};

#endif
