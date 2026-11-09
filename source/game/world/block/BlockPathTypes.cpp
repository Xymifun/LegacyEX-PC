#include "game/world/block/BlockPathTypes.hpp"

const BlockPathTypes* BlockPathTypes::BLOCKED = new BlockPathTypes(0, -1.0F);
const BlockPathTypes* BlockPathTypes::OPEN = new BlockPathTypes(1, 0.0F);
const BlockPathTypes* BlockPathTypes::WALKABLE = new BlockPathTypes(2, 0.0F);
const BlockPathTypes* BlockPathTypes::TRAPDOOR = new BlockPathTypes(3, 0.0F);
const BlockPathTypes* BlockPathTypes::FENCE = new BlockPathTypes(4, -1.0F);
const BlockPathTypes* BlockPathTypes::LAVA = new BlockPathTypes(5, -1.0F);
const BlockPathTypes* BlockPathTypes::WATER = new BlockPathTypes(6, 8.0F);
const BlockPathTypes* BlockPathTypes::WATER_BORDER = new BlockPathTypes(7, 8.0F);
const BlockPathTypes* BlockPathTypes::RAIL = new BlockPathTypes(8, 0.0F);
const BlockPathTypes* BlockPathTypes::DANGER_FIRE = new BlockPathTypes(9, 8.0F);
const BlockPathTypes* BlockPathTypes::DAMAGE_FIRE = new BlockPathTypes(10, 16.0F);
const BlockPathTypes* BlockPathTypes::DANGER_CACTUS = new BlockPathTypes(11, 8.0F);
const BlockPathTypes* BlockPathTypes::DAMAGE_CACTUS = new BlockPathTypes(12, -1.0F);
const BlockPathTypes* BlockPathTypes::DANGER_OTHER = new BlockPathTypes(13, 8.0F);
const BlockPathTypes* BlockPathTypes::DAMAGE_OTHER = new BlockPathTypes(14, -1.0F);
const BlockPathTypes* BlockPathTypes::DOOR_OPEN = new BlockPathTypes(15, 0.0F);
const BlockPathTypes* BlockPathTypes::DOOR_WOOD_CLOSED = new BlockPathTypes(16, -1.0F);
const BlockPathTypes* BlockPathTypes::DOOR_IRON_CLOSED = new BlockPathTypes(17, -1.0F);
const BlockPathTypes* BlockPathTypes::BREACH = new BlockPathTypes(18, 4.0F);

const BlockPathTypes* BlockPathTypes::VALUES[19] = {
    BLOCKED,      OPEN,      WALKABLE,         TRAPDOOR,         FENCE,         LAVA,          WATER,
    WATER_BORDER, RAIL,      DANGER_FIRE,      DAMAGE_FIRE,      DANGER_CACTUS, DAMAGE_CACTUS, DANGER_OTHER,
    DAMAGE_OTHER, DOOR_OPEN, DOOR_WOOD_CLOSED, DOOR_IRON_CLOSED, BREACH};

BlockPathTypes::BlockPathTypes(int id, float malus) : id(id), malus(malus)
{
}

int BlockPathTypes::ordinal() const
{
    return id;
}

float BlockPathTypes::getMalus() const
{
    return malus;
}
