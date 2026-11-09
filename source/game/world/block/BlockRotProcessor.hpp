#ifndef GAME_WORLD_BLOCK_BLOCKROTPROCESSOR_HPP
#define GAME_WORLD_BLOCK_BLOCKROTPROCESSOR_HPP

#include "game/util/Random.hpp"
#include "game/world/generation/StructurePlaceSettings.hpp"
#include "game/world/generation/StructureProcessor.hpp"

class BlockPos;

class BlockRotProcessor : public StructureProcessor
{
public:
    BlockRotProcessor(StructurePlaceSettings* settings, const BlockPos& position)
        : mIntegrity(settings->getIntegrity()),
          mRandom(settings->getRandom(&position, mOwnsRandom))
    {
    }
    virtual ~BlockRotProcessor()
    {
        if (mOwnsRandom)
            delete mRandom;
    }

    virtual StructureTemplate::StructureBlockInfo* processBlock(Level* level, const BlockPos& position,
                                                                StructureTemplate::StructureBlockInfo* blockInfo);

private:
    float mIntegrity;
    Random* mRandom;
    bool mOwnsRandom;
};

#endif
