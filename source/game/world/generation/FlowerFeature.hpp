#ifndef GAME_WORLD_GENERATION_FLOWERFEATURE_HPP
#define GAME_WORLD_GENERATION_FLOWERFEATURE_HPP

#include "game/world/generation/Feature.hpp"
#include "game/world/block/FlowerBlock.hpp"

class BlockState;

class FlowerFeature : public Feature
{
public:
    FlowerFeature(FlowerBlock* block, FlowerBlock::Flower* flower);
    virtual ~FlowerFeature();
    void setNextFlower(FlowerBlock* block, FlowerBlock::Flower* flower);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    FlowerBlock* mFlowerBlock;
    const BlockState* mState;
};

#endif
