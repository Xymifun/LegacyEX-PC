#ifndef GAME_WORLD_GENERATION_DOUBLEPLANTFEATURE_HPP
#define GAME_WORLD_GENERATION_DOUBLEPLANTFEATURE_HPP

#include "game/world/generation/Feature.hpp"
#include "game/world/block/DoublePlantBlock.hpp"

class BlockPos;
class Level;
class Random;

class DoublePlantFeature : public Feature
{
public:
    DoublePlantFeature();
    virtual ~DoublePlantFeature();
    void setNextPlantType(DoublePlantBlock::Variant* plantType);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    DoublePlantBlock::Variant* mNextPlantType;
};

#endif
