#ifndef GAME_CORE_COMMONSTATS_HPP
#define GAME_CORE_COMMONSTATS_HPP

#include "game/core/GenericStats.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CommonStats : public GenericStats
{
public:
    virtual ~CommonStats();

    virtual Stat* get_stat(int id);
    virtual Stat* get_walkOneM();
    virtual Stat* get_swimOneM();
    virtual Stat* get_fallOneM();
    virtual Stat* get_climbOneM();
    virtual Stat* get_minecartOneM();
    virtual Stat* get_boatOneM();
    virtual Stat* get_pigOneM();
    virtual Stat* get_portalsCreated();
    virtual Stat* get_cowsMilked();
    virtual Stat* get_netherLavaCollected();
    virtual Stat* get_killsZombie();
    virtual Stat* get_killsSkeleton();
    virtual Stat* get_killsCreeper();
    virtual Stat* get_killsSpider();
    virtual Stat* get_killsSpiderJockey();
    virtual Stat* get_killsZombiePigman();
    virtual Stat* get_killsSlime();
    virtual Stat* get_killsGhast();
    virtual Stat* get_killsNetherZombiePigman();
    virtual Stat* get_breedEntity(unsigned long long entityType);
    virtual Stat* get_tamedEntity(unsigned long long entityType);
    virtual Stat* get_craftedEntity(unsigned long long entityType);
    virtual Stat* get_shearedEntity(unsigned long long entityType);
    virtual Stat* get_totalBlocksMined();
    virtual Stat* get_timePlayed();
    virtual Stat* get_blocksPlaced(int blockId);
    virtual Stat* get_blocksMined(int blockId);
    virtual Stat* get_itemsCollected(int itemId, int auxValue);
    virtual Stat* get_itemsCrafted(int itemId);
    virtual Stat* get_itemsSmelted(int itemId);
    virtual Stat* get_itemsUsed(int itemId);
    virtual Stat* get_itemsBought(int itemId);
    virtual Stat* get_armourDyed(int itemId);
    virtual Stat* get_killsEnderdragon();
    virtual Stat* get_completeTheEnd();
    virtual Stat* get_enteredBiome(int biomeId);
    virtual Stat* get_achievement(eAward award);
    virtual Stat* get_MG03_Score();
    virtual Stat* get_MG03_RingCount();
    virtual Stat* get_MG03_BestTimeScoreChallenge();
    virtual Stat* get_MG03_BestTime();

    static arrayWithLength<unsigned char> makeParam(int value);

    virtual arrayWithLength<unsigned char> getParam_MG03Score(int value);
    virtual arrayWithLength<unsigned char> getParam_MG03RingCount(int value);
    virtual arrayWithLength<unsigned char> getParam_MG03BestTime(unsigned int value);
    virtual arrayWithLength<unsigned char> getParam_walkOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_swimOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_fallOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_climbOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_minecartOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_boatOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_pigOneM(int value);
    virtual arrayWithLength<unsigned char> getParam_blocksMined(int blockId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsCollected(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsCrafted(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsSmelted(int itemId, int auxValue, int count);
    arrayWithLength<unsigned char> getParam_itemsUsed(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_itemsBought(int itemId, int auxValue, int count);
    virtual arrayWithLength<unsigned char> getParam_time(int value);
    virtual arrayWithLength<unsigned char> getParam_noArgs();

    static unsigned int readParam(arrayWithLength<unsigned char> value);
};

#endif
