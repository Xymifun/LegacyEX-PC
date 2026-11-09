#ifndef GAME_WORLD_GENERATION_MONSTERROOMFEATURE_HPP
#define GAME_WORLD_GENERATION_MONSTERROOMFEATURE_HPP

#include <string>

#include "game/world/generation/Feature.hpp"

class BlockPos;
class Level;
class Random;

class MonsterRoomFeature : public Feature
{
public:
    virtual ~MonsterRoomFeature() {}
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

    std::wstring randomEntityId(Random& random);
};

#endif
