#include "game/world/generation/IceSpikeFeature.hpp"

#include <cstdlib>

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

IceSpikeFeature::~IceSpikeFeature()
{
}

bool IceSpikeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos pos = origin;
    while (level->isEmptyBlock(pos) && pos.y > 2)
    {
        pos = pos.below();
    }

    if (level->getBlockState(pos)->getBlock() != Blocks::snow)
    {
        return false;
    }

    pos = pos.above(random.nextInt(4));
    int height = random.nextInt(4) + 7;
    int width = height / 4 + random.nextInt(2);

    if (width > 1 && random.nextInt(60) == 0)
    {
        pos = pos.above(random.nextInt(30) + 10);
    }

    for (int y = 0; y < height; ++y)
    {
        float radius = (1.0f - (float)y / (float)height) * (float)width;
        int extent = Mth::ceil(radius);

        for (int x = -extent; x <= extent; ++x)
        {
            float dx = (float)Mth::abs(x) - 0.25f;

            for (int z = -extent; z <= extent; ++z)
            {
                float dz = (float)Mth::abs(z) - 0.25f;
                if (((x == 0 && z == 0) || dx * dx + dz * dz <= radius * radius)
                    && ((x != -extent && x != extent && z != -extent && z != extent) || random.nextFloat() <= 0.75f))
                {
                    const BlockState* state = level->getBlockState(pos.offset(x, y, z));
                    Block* block = state->getBlock();
                    if (state->getMaterial() == Material::AIR || block == Blocks::dirt || block == Blocks::snow
                        || block == Blocks::ice)
                    {
                        setBlock(level, pos.offset(x, y, z), Blocks::packed_ice->defaultBlockState(),
                                 Block::UPDATE_NONE);
                    }

                    if (y != 0 && extent > 1)
                    {
                        state = level->getBlockState(pos.offset(x, -y, z));
                        block = state->getBlock();
                        if (state->getMaterial() == Material::AIR || block == Blocks::dirt || block == Blocks::snow
                            || block == Blocks::ice)
                        {
                            setBlock(level, pos.offset(x, -y, z), Blocks::packed_ice->defaultBlockState(),
                                     Block::UPDATE_NONE);
                        }
                    }
                }
            }
        }
    }

    int flare = width - 1;
    if (flare < 0)
    {
        flare = 0;
    }
    else if (flare > 1)
    {
        flare = 1;
    }

    for (int x = -flare; x <= flare; ++x)
    {
        for (int z = -flare; z <= flare; ++z)
        {
            BlockPos target = pos.offset(x, -1, z);
            int remaining = 50;
            if (std::abs(x) == 1 && std::abs(z) == 1)
            {
                remaining = random.nextInt(5);
            }

            while (target.y > 50)
            {
                const BlockState* state = level->getBlockState(target);
                Block* block = state->getBlock();
                if (state->getMaterial() != Material::AIR && block != Blocks::dirt && block != Blocks::snow
                    && block != Blocks::ice && block != Blocks::packed_ice)
                {
                    break;
                }

                setBlock(level, target, Blocks::packed_ice->defaultBlockState(), Block::UPDATE_NONE);
                target = target.below();
                --remaining;
                if (remaining <= 0)
                {
                    target = target.below(random.nextInt(5) + 1);
                    remaining = random.nextInt(5);
                }
            }
        }
    }

    return true;
}
