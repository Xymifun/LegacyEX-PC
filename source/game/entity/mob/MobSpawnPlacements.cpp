#include "game/entity/mob/MobSpawnPlacements.hpp"
#include "game/core/EntityType.hpp"

#include <boost/unordered_map.hpp>

static boost::unordered_map<unsigned long long, Mob::SpawnPlacementType> placementTypeCtor()
{
    boost::unordered_map<unsigned long long, Mob::SpawnPlacementType> placements;

    placements[EntityType::BAT] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::CHICKEN] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::COW] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::HORSE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SKELETON_HORSE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::ZOMBIE_HORSE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::DONKEY] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::MULE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::MUSHROOM_COW] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::OCELOT] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::PIG] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::RABBIT] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SHEEP] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::PARROT] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SNOW_MAN] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SQUID] = Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER;
    placements[EntityType::TURTLE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::FISH] = Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER;
    placements[EntityType::PUFFERFISH] = Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER;
    placements[EntityType::SALMON] = Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER;
    placements[EntityType::TROPICAL_FISH] = Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER;
    placements[EntityType::VILLAGER_GOLEM] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::WOLF] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::VILLAGER] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::ENDER_DRAGON] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::WITHER_BOSS] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::BLAZE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::CAVE_SPIDER] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::CREEPER] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::ENDER_MAN] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::ENDERMITE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::GHAST] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::GIANT] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::GUARDIAN] = Mob::SPAWN_PLACEMENT_TYPE_IN_WATER;
    placements[EntityType::MAGMA_CUBE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::PIG_ZOMBIE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SILVERFISH] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SKELETON] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::WITHER_SKELETON] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::STRAY] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SLIME] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::SPIDER] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::WITCH] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::ZOMBIE] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::ZOMBIE_VILLAGER] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::HUSK] = Mob::SPAWN_PLACEMENT_TYPE_ON_GROUND;
    placements[EntityType::DROWNED] = Mob::SPAWN_PLACEMENT_TYPE_ON_WATER_FLOOR;
    placements[EntityType::DOLPHIN] = Mob::SPAWN_PLACEMENT_TYPE_IN_OPEN_WATER;

    return placements;
}

static boost::unordered_map<unsigned long long, Mob::SpawnPlacementType> placementType = placementTypeCtor();

Mob::SpawnPlacementType MobSpawnPlacements::getPlacementType(unsigned long long entityType)
{
    return placementType[entityType];
}
