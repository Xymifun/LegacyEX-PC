#ifndef GAME_CORE_STATS_HPP
#define GAME_CORE_STATS_HPP

#include "game/core/Stat.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Stats
{
public:
    static Stat* walkOneM;
    static Stat* swimOneM;
    static Stat* fallOneM;
    static Stat* climbOneM;
    static Stat* minecartOneM;
    static Stat* boatOneM;
    static Stat* pigOneM;
    static Stat* portalsUsed;
    static Stat* cowsMilked;
    static Stat* netherLavaCollected;
    static Stat* killsZombie;
    static Stat* killsSkeleton;
    static Stat* killsCreeper;
    static Stat* killsSpider;
    static Stat* killsSpiderJockey;
    static Stat* killsZombiePigman;
    static Stat* killsSlime;
    static Stat* killsGhast;
    static Stat* killsNetherZombiePigman;
    static Stat* mg03Score;
    static Stat* mg03RingCount;
    static Stat* mg03BestTimeScoreChallenge;
    static Stat* mg03BestTime;
    static Stat* befriendsWolf;
    static Stat* totalBlocksMined;
    static Stat* timePlayed;
    static Stat* killsEnderdragon;
    static Stat* completeTheEnd;

    static bool blockStatsBuilt;
    static bool itemStatsBuilt;
    static bool craftableStatsBuilt;

    static arrayWithLength<Stat*> blockMinedStats;
    static arrayWithLength<Stat*> itemCollectedStats;
    static arrayWithLength<Stat*> itemCraftedStats;
    static arrayWithLength<Stat*> blockPlacedStats;
    static arrayWithLength<Stat*> biomesVisitedStats;
    static arrayWithLength<Stat*> rainbowCollectionStats;
    static arrayWithLength<Stat*> armourDyedStats;

    static void init();
    static void staticCtor();
    static void buildBlockStats();
    static void buildItemStats();
    static void buildCraftableStats();
    static void buildAdditionalStats();
    static Stat* get(int id);
};

#endif
