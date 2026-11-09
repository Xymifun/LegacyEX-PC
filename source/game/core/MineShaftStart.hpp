#ifndef GAME_CORE_MINESHAFTSTART_HPP
#define GAME_CORE_MINESHAFTSTART_HPP

#include "game/world/generation/StructureStart.hpp"
#include "game/world/generation/MineShaftFeature.hpp"

class Level;
class Random;

class MineShaftStart : public StructureStart
{
public:
    MineShaftStart(Level* level, Random& random, int chunkX, int chunkZ, MineShaftFeature::MineShaftType type);
    MineShaftStart();

    virtual EStructureStart GetType();

    static StructureStart* Create();

private:
    MineShaftFeature::MineShaftType mType;
};

#endif
