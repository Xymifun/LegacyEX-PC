#ifndef GAME_ENTITY_MOB_MOBCATEGORY_HPP
#define GAME_ENTITY_MOB_MOBCATEGORY_HPP

#include "game/world/biome/arrayWithLength.hpp"

class Material;

class MobCategory
{
public:
    static MobCategory* MONSTER;
    static MobCategory* CREATURE;
    static MobCategory* AMBIENT;
    static MobCategory* WATER_CREATURE;
    static MobCategory* SQUID;
    static MobCategory* WOLF;
    static MobCategory* CHICKEN;
    static MobCategory* MUSHROOM_COW;
    static MobCategory* SLIME;
    static MobCategory* GUARDIAN;
    static MobCategory* VILLAGER;
    static MobCategory* SNOW_MAN;
    static MobCategory* VILLAGER_GOLEM;
    static MobCategory* WITHER_BOSS;
    static MobCategory* ARMOR_STAND;
    static MobCategory* ENDER_CRYSTAL;
    static MobCategory* DOLPHIN;
    static MobCategory* PHANTOM;

    static arrayWithLength<MobCategory*> CATEGORIES;

    MobCategory(int maxInstancesPerChunk, Material* spawnPositionMaterial, bool friendly, bool persistent,
                unsigned long long enumBaseClass, bool singleType);

    static void staticCtor();

    static MobCategory* getCategory(unsigned long long enumBaseClass);

    int getMaxInstancesPerChunk();
    unsigned int getMaxInstancesPerLevel();
    int getMaxInstancesPerLevelWithBreeding();
    int getMaxInstancesPerLevelWithSpawnEgg();
    Material* getSpawnPositionMaterial();
    unsigned long long getEnumBaseClass();
    bool isFriendly();
    bool isPersistent();
    bool isSingleType();

private:
    static int TOTAL_MAX_INSTANCES;

    int maxInstancesPerChunk;
    int maxInstancesPerLevel;
    int maxInstancesPerLevelWithBreeding;
    int maxInstancesPerLevelWithSpawnEgg;
    Material* spawnPositionMaterial;
    bool friendly;
    bool persistent;
    bool singleType;
    unsigned long long enumBaseClass;
};

#endif
