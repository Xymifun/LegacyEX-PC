#include <cstddef>
#include "game/core/EntityType.hpp"

#include "game/entity/mob/MobCategory.hpp"
#include "game/world/block/Material.hpp"

static const int NUM_MOB_CATEGORIES = 18;

struct MobSpawnLimits
{
    unsigned long long entityType;
    int maxInstancesPerLevel;
    int extraInstancesForBreeding;
    int extraInstancesForSpawnEgg;
};

#ifdef MINECRAFT_PC
static MobSpawnLimits SPAWN_LIMITS[NUM_MOB_CATEGORIES] = {{0x40ULL, 50, 0, 20},
                                                          {EntityType::CREATURE_CATEGORY, 50, 200, 50},
                                                          {EntityType::AMBIENT_CREATURE, 20, 0, 8},
                                                          {EntityType::WATER_ANIMAL, 20, 0, 8},
                                                          {EntityType::SQUID, 8, 0, 6},
                                                          {EntityType::WOLF, 8, 56, 16},
                                                          {EntityType::CHICKEN, 8, 56, 16},
                                                          {EntityType::MUSHROOM_COW, 2, 48, 16},
                                                          {EntityType::SLIME, 25, 0, 0},
                                                          {EntityType::GUARDIAN, 35, 0, 0},
                                                          {EntityType::VILLAGER, 35, 65, 15},
                                                          {EntityType::SNOW_MAN, 16, 0, 0},
                                                          {EntityType::VILLAGER_GOLEM, 16, 0, 0},
                                                          {EntityType::WITHER_BOSS, 5, 0, 0},
                                                          {EntityType::ARMOR_STAND, 16, 0, 0},
                                                          {EntityType::ENDER_CRYSTAL, 64, 0, 0},
                                                          {EntityType::DOLPHIN, 15, 5, 5},
                                                          {EntityType::PHANTOM, 5, 0, 0}};
#else
static MobSpawnLimits SPAWN_LIMITS[NUM_MOB_CATEGORIES] = {{0x40ULL, 50, 0, 20},
                                                          {EntityType::CREATURE_CATEGORY, 50, 20, 20},
                                                          {EntityType::AMBIENT_CREATURE, 20, 0, 8},
                                                          {EntityType::WATER_ANIMAL, 20, 0, 8},
                                                          {EntityType::SQUID, 8, 0, 6},
                                                          {EntityType::WOLF, 8, 8, 10},
                                                          {EntityType::CHICKEN, 8, 8, 10},
                                                          {EntityType::MUSHROOM_COW, 2, 10, 8},
                                                          {EntityType::SLIME, 25, 0, 0},
                                                          {EntityType::GUARDIAN, 35, 0, 0},
                                                          {EntityType::VILLAGER, 35, 0, 15},
                                                          {EntityType::SNOW_MAN, 16, 0, 0},
                                                          {EntityType::VILLAGER_GOLEM, 16, 0, 0},
                                                          {EntityType::WITHER_BOSS, 5, 0, 0},
                                                          {EntityType::ARMOR_STAND, 16, 0, 0},
                                                          {EntityType::ENDER_CRYSTAL, 64, 0, 0},
                                                          {EntityType::DOLPHIN, 15, 5, 5},
                                                          {EntityType::PHANTOM, 5, 0, 0}};
#endif

MobCategory* MobCategory::MONSTER;
MobCategory* MobCategory::CREATURE;
MobCategory* MobCategory::AMBIENT;
MobCategory* MobCategory::WATER_CREATURE;
MobCategory* MobCategory::SQUID;
MobCategory* MobCategory::WOLF;
MobCategory* MobCategory::CHICKEN;
MobCategory* MobCategory::MUSHROOM_COW;
MobCategory* MobCategory::SLIME;
MobCategory* MobCategory::GUARDIAN;
MobCategory* MobCategory::VILLAGER;
MobCategory* MobCategory::SNOW_MAN;
MobCategory* MobCategory::VILLAGER_GOLEM;
MobCategory* MobCategory::WITHER_BOSS;
MobCategory* MobCategory::ARMOR_STAND;
MobCategory* MobCategory::ENDER_CRYSTAL;
MobCategory* MobCategory::DOLPHIN;
MobCategory* MobCategory::PHANTOM;

int MobCategory::TOTAL_MAX_INSTANCES;

arrayWithLength<MobCategory*> MobCategory::CATEGORIES(NUM_MOB_CATEGORIES, true);

MobCategory::MobCategory(int maxInstancesPerChunk, Material* spawnPositionMaterial, bool friendly, bool persistent,
                         unsigned long long enumBaseClass, bool singleType)
{
    this->maxInstancesPerChunk = maxInstancesPerChunk;
    this->spawnPositionMaterial = spawnPositionMaterial;
    this->friendly = friendly;
    this->persistent = persistent;
    this->singleType = singleType;
    this->enumBaseClass = enumBaseClass;

    for (int i = 0; i < NUM_MOB_CATEGORIES; i++)
    {
        if (enumBaseClass == SPAWN_LIMITS[i].entityType)
        {
            this->maxInstancesPerLevel = SPAWN_LIMITS[i].maxInstancesPerLevel;
            this->maxInstancesPerLevelWithBreeding = this->maxInstancesPerLevel
                                                     + SPAWN_LIMITS[i].extraInstancesForBreeding;
            this->maxInstancesPerLevelWithSpawnEgg = this->maxInstancesPerLevelWithBreeding
                                                     + SPAWN_LIMITS[i].extraInstancesForSpawnEgg;
        }
    }
}

int MobCategory::getMaxInstancesPerChunk()
{
    return maxInstancesPerChunk;
}

void MobCategory::staticCtor()
{
    MONSTER = new MobCategory(70, Material::AIR, false, false, 0x40ULL, false);
    CREATURE = new MobCategory(10, Material::AIR, true, true, EntityType::CREATURE_CATEGORY, false);
    AMBIENT = new MobCategory(15, Material::AIR, true, false, EntityType::AMBIENT_CREATURE, false);
    WATER_CREATURE = new MobCategory(5, Material::WATER, true, false, EntityType::WATER_ANIMAL, false);

    CATEGORIES.data[0] = MONSTER;
    CATEGORIES.data[1] = CREATURE;
    CATEGORIES.data[2] = AMBIENT;
    CATEGORIES.data[3] = WATER_CREATURE;

    SQUID = new MobCategory(4, Material::WATER, true, false, EntityType::SQUID, true);
    WOLF = new MobCategory(3, Material::AIR, true, true, EntityType::WOLF, true);
    CHICKEN = new MobCategory(2, Material::AIR, true, true, EntityType::CHICKEN, true);
    MUSHROOM_COW = new MobCategory(2, Material::AIR, true, true, EntityType::MUSHROOM_COW, true);
    SLIME = new MobCategory(15, Material::AIR, false, false, EntityType::SLIME, false);
    GUARDIAN = new MobCategory(25, Material::WATER, false, false, EntityType::GUARDIAN, false);

    CATEGORIES.data[4] = SQUID;
    CATEGORIES.data[5] = WOLF;
    CATEGORIES.data[6] = CHICKEN;
    CATEGORIES.data[7] = MUSHROOM_COW;
    CATEGORIES.data[8] = SLIME;
    CATEGORIES.data[9] = GUARDIAN;

    VILLAGER = new MobCategory(25, Material::AIR, false, false, EntityType::VILLAGER, true);
    CATEGORIES.data[10] = VILLAGER;

    SNOW_MAN = new MobCategory(25, Material::AIR, false, false, EntityType::SNOW_MAN, true);
    CATEGORIES.data[11] = SNOW_MAN;

    VILLAGER_GOLEM = new MobCategory(25, Material::AIR, false, false, EntityType::VILLAGER_GOLEM, true);
    CATEGORIES.data[12] = VILLAGER_GOLEM;

    WITHER_BOSS = new MobCategory(25, Material::AIR, false, false, EntityType::WITHER_BOSS, true);
    CATEGORIES.data[13] = WITHER_BOSS;

    ARMOR_STAND = new MobCategory(25, Material::AIR, false, false, EntityType::ARMOR_STAND, true);
    CATEGORIES.data[14] = ARMOR_STAND;

    ENDER_CRYSTAL = new MobCategory(25, Material::AIR, false, false, EntityType::ENDER_CRYSTAL, true);
    CATEGORIES.data[15] = ENDER_CRYSTAL;

    DOLPHIN = new MobCategory(25, Material::WATER, false, false, EntityType::DOLPHIN, true);
    CATEGORIES.data[16] = DOLPHIN;

    PHANTOM = new MobCategory(5, Material::AIR, false, false, EntityType::PHANTOM, true);
    CATEGORIES.data[17] = PHANTOM;

    for (int i = 0; i < NUM_MOB_CATEGORIES; i++)
    {
        TOTAL_MAX_INSTANCES += CATEGORIES.data[i]->getMaxInstancesPerLevelWithSpawnEgg();
    }
}

MobCategory* MobCategory::getCategory(unsigned long long enumBaseClass)
{
    switch (enumBaseClass)
    {
    case EntityType::VILLAGER:
        return VILLAGER;
    case EntityType::CHICKEN:
        return CHICKEN;
    case EntityType::SQUID:
        return SQUID;
    case EntityType::WOLF:
        return WOLF;
    case EntityType::MUSHROOM_COW:
        return MUSHROOM_COW;
    case EntityType::SNOW_MAN:
        return SNOW_MAN;
    case EntityType::VILLAGER_GOLEM:
        return VILLAGER_GOLEM;
    case EntityType::WITHER_BOSS:
        return WITHER_BOSS;
    case EntityType::ARMOR_STAND:
        return ARMOR_STAND;
    case EntityType::ENDER_CRYSTAL:
        return ENDER_CRYSTAL;
    case EntityType::DOLPHIN:
        return DOLPHIN;
    case EntityType::PHANTOM:
        return PHANTOM;
    default:
        break;
    }

    if ((enumBaseClass & EntityType::CREATURE_CATEGORY) == EntityType::CREATURE_CATEGORY)
    {
        return CREATURE;
    }

    if ((enumBaseClass & 0x40ULL) == 0x40ULL)
    {
        return MONSTER;
    }

    if (eTYPE_DERIVED_FROM(EntityType::SLIME, enumBaseClass))
    {
        return SLIME;
    }

    if (eTYPE_DERIVED_FROM(EntityType::GUARDIAN, enumBaseClass))
    {
        return GUARDIAN;
    }

    if (eTYPE_DERIVED_FROM(EntityType::ENEMY, enumBaseClass))
    {
        return MONSTER;
    }

    if ((enumBaseClass & EntityType::AMBIENT_CREATURE) == EntityType::AMBIENT_CREATURE)
    {
        return AMBIENT;
    }

    MobCategory* category = NULL;
    if ((enumBaseClass & EntityType::WATER_ANIMAL) == EntityType::WATER_ANIMAL)
    {
        category = WATER_CREATURE;
    }

    return category;
}

unsigned int MobCategory::getMaxInstancesPerLevel()
{
    return maxInstancesPerLevel;
}

int MobCategory::getMaxInstancesPerLevelWithBreeding()
{
    return maxInstancesPerLevelWithBreeding;
}

int MobCategory::getMaxInstancesPerLevelWithSpawnEgg()
{
    return maxInstancesPerLevelWithSpawnEgg;
}

Material* MobCategory::getSpawnPositionMaterial()
{
    return spawnPositionMaterial;
}

unsigned long long MobCategory::getEnumBaseClass()
{
    return enumBaseClass;
}

bool MobCategory::isFriendly()
{
    return friendly;
}

bool MobCategory::isPersistent()
{
    return persistent;
}

bool MobCategory::isSingleType()
{
    return singleType;
}
