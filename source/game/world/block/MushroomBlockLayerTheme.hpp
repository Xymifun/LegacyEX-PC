#ifndef GAME_WORLD_BLOCK_MUSHROOMBLOCKLAYERTHEME_HPP
#define GAME_WORLD_BLOCK_MUSHROOMBLOCKLAYERTHEME_HPP

#include "game/world/generation/LayerTheme.hpp"

class Block;
class BlockTransaction;
class HugeMushroomBlock;
class Random;

class MushroomBlockLayerTheme : public LayerTheme
{
public:
    MushroomBlockLayerTheme(HugeMushroomBlock* block);

    virtual void Apply(Random& random, BlockTransaction* transaction, int& minY, int& maxY) const;

private:
    Block* mBlock;
};

#endif
