#ifndef GAME_WORLD_BLOCK_BLOCKPATHTYPES_HPP
#define GAME_WORLD_BLOCK_BLOCKPATHTYPES_HPP

class BlockPathTypes
{
public:
    static const BlockPathTypes* BLOCKED;
    static const BlockPathTypes* OPEN;
    static const BlockPathTypes* WALKABLE;
    static const BlockPathTypes* TRAPDOOR;
    static const BlockPathTypes* FENCE;
    static const BlockPathTypes* LAVA;
    static const BlockPathTypes* WATER;
    static const BlockPathTypes* WATER_BORDER;
    static const BlockPathTypes* RAIL;
    static const BlockPathTypes* DANGER_FIRE;
    static const BlockPathTypes* DAMAGE_FIRE;
    static const BlockPathTypes* DANGER_CACTUS;
    static const BlockPathTypes* DAMAGE_CACTUS;
    static const BlockPathTypes* DANGER_OTHER;
    static const BlockPathTypes* DAMAGE_OTHER;
    static const BlockPathTypes* DOOR_OPEN;
    static const BlockPathTypes* DOOR_WOOD_CLOSED;
    static const BlockPathTypes* DOOR_IRON_CLOSED;
    static const BlockPathTypes* BREACH;

    BlockPathTypes(int id, float malus);

    int ordinal() const;
    float getMalus() const;

public:
    static const BlockPathTypes* VALUES[19];

    int id;
    float malus;
};

#endif
