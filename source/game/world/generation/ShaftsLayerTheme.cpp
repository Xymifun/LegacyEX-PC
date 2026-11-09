#include "game/world/generation/ShaftsLayerTheme.hpp"

#include <algorithm>

#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

ShaftsLayerTheme::ShaftsLayerTheme(int heightOffset, const std::vector<const BlockState*>& states)
    : mHeightOffset(heightOffset),
      mStates(states)
{
}

bool ShaftsLayerTheme::hasShaft(int x, int z) const
{
    return x % 5 == 0 && z % 5 == 0;
}

void ShaftsLayerTheme::Apply(Random&, BlockTransaction* transaction, int& width, int& depth) const
{
    if (mStates.empty())
    {
        return;
    }

    const int halfWidth = transaction->mSize.x / 2;
    const int halfDepth = transaction->mSize.z / 2;
    const int maxIndex = static_cast<int>(mStates.size()) - 1;
    for (int x = 0; x < transaction->mSize.x; ++x)
    {
        for (int z = 0; z < transaction->mSize.z; ++z)
        {
            if (hasShaft(x + halfWidth, z + halfDepth))
            {
                const int surface = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z);
                if (surface != -1)
                {
                    for (int y = std::min(mHeightOffset + surface, transaction->mSize.y - 1); y > 0; --y)
                    {
                        const BlockState* state = mStates.at(std::min(maxIndex, mHeightOffset + surface - y));
                        if (state != NULL)
                        {
                            transaction->setBlock(BlockPos(x, y, z), state);
                        }
                    }
                }
            }
        }
    }
}
