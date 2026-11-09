#include "game/world/generation/SeaAnemoneFeature.hpp"

#include "game/core/CoralFan.hpp"
#include "game/core/Coral.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"

class Property;


SeaAnemoneFeature::~SeaAnemoneFeature()
{
}

bool SeaAnemoneFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (!level->mIsClientSide)
    {
        BlockPos pos = origin;
        while (!level->isSolidBlockingBlock(pos.below()) && pos.y > 48)
        {
            pos = pos.below();
        }

        if (!level->isEmptyWaterBlock(pos) || !level->isSolidBlockingBlock(pos.below()) || pos.y <= 48
            || !level->isUnderWater(pos, 259, 0) || !level->reallyHasChunkAt(pos))
        {
            return false;
        }

        if (random.rollPercentageChance(75))
        {
            const BlockState* state = Blocks::coral->defaultBlockState()->setValue(
                Coral::VARIANT, random.nextInt(Coral::VARIANT->getValueCount()));
            setBlock(level, pos, state, Block::UPDATE_NONE);
        }
        else
        {
            const BlockState* state =
                Blocks::coral_fan->defaultBlockState()
                    ->setValue(CoralFan::VARIANT, random.nextInt(CoralFan::VARIANT->getValueCount()))
                    ->setValue(CoralFan::DIRECTION, random.nextInt(CoralFan::DIRECTION->getValueCount()));
            setBlock(level, pos, state, Block::UPDATE_NONE);
        }
    }
    return true;
}
