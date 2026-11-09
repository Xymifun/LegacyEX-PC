#include "game/world/generation/DoublePlantFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"

DoublePlantFeature::DoublePlantFeature() : Feature(false)
{
}

DoublePlantFeature::~DoublePlantFeature()
{
}

void DoublePlantFeature::setNextPlantType(DoublePlantBlock::Variant* plantType)
{
    mNextPlantType = plantType;
}

bool DoublePlantFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    bool placed = false;

    for (int attempt = 0; attempt < 64; ++attempt)
    {
        const BlockPos pos = origin.offset(random.nextInt(8) - random.nextInt(8), random.nextInt(4) - random.nextInt(4),
                                           random.nextInt(8) - random.nextInt(8));
        if (level->isEmptyBlock(pos) && ((!level->mDimension->isHasCeiling()) || pos.y < 126)
            && static_cast<DoublePlantBlock*>(Blocks::double_plant)->mayPlace(level, pos))
        {
            static_cast<DoublePlantBlock*>(Blocks::double_plant)->placeAt(level, pos, mNextPlantType, 2);
            placed = true;
        }
    }

    return placed;
}
