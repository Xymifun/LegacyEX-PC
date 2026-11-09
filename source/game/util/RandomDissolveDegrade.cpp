#include "game/util/RandomDissolveDegrade.hpp"

#include "game/util/Random.hpp"
#include "game/util/AABB.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockDegradeRoutine.hpp"

int randomBetween(Random& random, int min, int max);

void RandomDissolveDegrade::NextBlocks(BlockDegradeRoutine::LayerCache* layerCache, arrayWithLength<BlockPos>& blocks)
{
    Random random;
    AABB* bounds = layerCache->getCurrentBounds();
    unsigned int blockCount = 0;

    for (unsigned int attempts = 0; blockCount < blocks.length && attempts < 1000; ++attempts)
    {
        int x = randomBetween(random, static_cast<int>(bounds->mMinX), static_cast<int>(bounds->mMaxX) + 1);
        int y = randomBetween(random, static_cast<int>(bounds->mMinY), static_cast<int>(bounds->mMaxY) + 1);
        int z = randomBetween(random, static_cast<int>(bounds->mMinZ), static_cast<int>(bounds->mMaxZ) + 1);
        BlockPos position(x, y, z);

        if (!layerCache->isBroken(position))
        {
            blocks.data[blockCount] = position;
            ++blockCount;
        }
    }
}

ConsoleGameRules::EGameRuleType RandomDissolveDegrade::getActionType()
{
    return ConsoleGameRules::eGameRuleType_RandomDissolveDegrade;
}
