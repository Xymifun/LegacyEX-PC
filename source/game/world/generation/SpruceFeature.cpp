#include "game/world/generation/SpruceFeature.hpp"

#include <cstdlib>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"


static const BlockState* spruceTrunkState = NULL;
static const BlockState* spruceLeafState = NULL;

SpruceFeature::SpruceFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 6, spruceTrunkState, spruceLeafState, false)
{
}

void SpruceFeature::staticCtor()
{
    spruceTrunkState = Blocks::log->defaultBlockState()->setValue(OldLogBlock::VARIANT, PlanksBlock::Variant::SPRUCE);
    spruceLeafState = Blocks::leaves->defaultBlockState()
                          ->setValue(OldLeafBlock::VARIANT, PlanksBlock::Variant::SPRUCE)
                          ->setValue(LeafBlock::CHECK_DECAY, false);
}

bool SpruceFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    int height = random.nextInt(4) + mBaseHeight;
    int bareTrunk = random.nextInt(2) + 1;
    int foliageHeight = height - bareTrunk;
    int maxRadius = random.nextInt(2) + 2;
    bool free = true;

    if (pos.y < 1 || pos.y + height + 1 > 256)
        return false;

    if (gApp.getLevelGenerationOptions() != NULL)
    {
        LevelGenerationOptions* levelGenerationOptions = gApp.getLevelGenerationOptions();
        if (levelGenerationOptions->checkIntersects(pos.offset(-maxRadius, -1, -maxRadius),
                                                    pos.offset(maxRadius, height, maxRadius)))
            return false;
    }

    MutableBlockPos checkPos;
    for (int y = pos.y; y <= pos.y + height + 1 && free; ++y)
    {
        int radius = y - pos.y >= bareTrunk ? maxRadius : 0;
        for (int x = pos.x - radius; x <= pos.x + radius && free; ++x)
        {
            for (int z = pos.z - radius; z <= pos.z + radius && free; ++z)
            {
                if (y >= 0 && y < 256)
                {
                    checkPos.set(x, y, z);
                    Material* material = level->getBlockState(checkPos)->getMaterial();
                    if (material != Material::AIR && material != Material::LEAVES)
                        free = false;
                }
                else
                {
                    free = false;
                }
            }
        }
    }

    if (!free)
        return false;

    Block* ground = level->getBlockState(pos.below())->getBlock();
    if ((ground != Blocks::grass && ground != Blocks::dirt && ground != Blocks::farmland) || pos.y >= 255 - height)
        return false;

    setDirtAt(level, pos.below());
    if (!placeTrunk(level, pos, random, height - random.nextInt(3)))
        return true;

    int radius = random.nextInt(2);
    int radiusLimit = 1;
    int nextRadius = 0;
    for (int layer = 0; layer <= foliageHeight; ++layer)
    {
        int y = pos.y + height - layer;
        for (int x = pos.x - radius; x <= pos.x + radius; ++x)
        {
            int xOffset = x - pos.x;
            for (int z = pos.z - radius; z <= pos.z + radius; ++z)
            {
                int zOffset = z - pos.z;
                bool corner = abs(xOffset) == radius && abs(zOffset) == radius && radius > 0;
                if (!corner)
                {
                    const BlockPos& leafPos = BlockPos(x, y, z);
                    if (!level->getBlockState(leafPos)->isSolid())
                        setBlock(level, leafPos, mLeafState, Block::UPDATE_NONE);
                }
            }
        }

        if (radius >= radiusLimit)
        {
            radius = nextRadius;
            nextRadius = 1;
            ++radiusLimit;
            if (radiusLimit > maxRadius)
                radiusLimit = maxRadius;
        }
        else
        {
            ++radius;
        }
    }

    return true;
}
