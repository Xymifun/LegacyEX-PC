#ifndef GAME_WORLD_GENERATION_OREFEATURE_HPP
#define GAME_WORLD_GENERATION_OREFEATURE_HPP

#include "game/world/generation/Feature.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/biome/Biome.hpp"

class BlockPos;
class BlockState;
class Level;
class Random;

class OreFeature : public Feature
{
public:
    class NaturalStonePredicate : public Predicate<const Biome::BlockPair&>
    {
    public:
        virtual bool apply(const Biome::BlockPair& pair) const;
    };

    OreFeature(const BlockState* state, int numberOfBlocks);
    OreFeature(const BlockState* state, int numberOfBlocks, const Predicate<const Biome::BlockPair&>* predicate);
    virtual ~OreFeature();

    void _init(const BlockState* state, int numberOfBlocks, const Predicate<const Biome::BlockPair&>* predicate);
    virtual bool place(Level* level, Random& random, const BlockPos& origin);

private:
    int mBlockId;
    int mBlockData;
    int mNumberOfBlocks;
    const Predicate<const Biome::BlockPair&>* mPredicate;
};

#endif
