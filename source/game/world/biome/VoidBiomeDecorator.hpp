#ifndef GAME_WORLD_BIOME_VOIDBIOMEDECORATOR_HPP
#define GAME_WORLD_BIOME_VOIDBIOMEDECORATOR_HPP

#include "game/world/biome/BiomeDecorator.hpp"

class Biome;
class Level;
class Random;

class VoidBiomeDecorator : public BiomeDecorator
{
public:
    virtual void decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin);
};

#endif
