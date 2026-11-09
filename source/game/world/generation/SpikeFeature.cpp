#include "game/world/generation/SpikeFeature.hpp"

#include <cstdlib>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"

SpikeFeature::SpikeFeature() : Feature(false)
{
    mSpike = NULL;
    mCrystalBeamTarget = NULL;
    mCrystalInvulnerable = false;
}

SpikeFeature::~SpikeFeature()
{
    delete mCrystalBeamTarget;
}

void SpikeFeature::setSpike(EndSpike* spike)
{
    mSpike = spike;
}

void SpikeFeature::setCrystalInvulnerable(bool invulnerable)
{
    mCrystalInvulnerable = invulnerable;
}

bool SpikeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (mSpike == NULL)
    {
        return false;
    }

    int radius = (int)mSpike->getRadius();
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(
        BlockPos(origin.x - radius, 1, origin.z - radius),
        BlockPos(origin.x + radius, (int)mSpike->getHeight() + 10, origin.z + radius));

    while (positions->hasNext())
    {
        MutableBlockPos position = *positions->next();
        if (position.distSqr((double)origin.x, (double)position.y, (double)origin.z) <= (double)(radius * radius + 1)
            && position.y < (int)mSpike->getHeight())
        {
            setBlock(level, position, Blocks::obsidian->defaultBlockState(), Block::UPDATE_NONE);
        }
        else if (position.y > 65)
        {
            setBlock(level, position, Blocks::air->defaultBlockState(), Block::UPDATE_NONE);
        }
    }
    delete positions;

    if (mSpike->isGuarded())
    {
        for (int x = -2; x <= 2; ++x)
        {
            for (int z = -2; z <= 2; ++z)
            {
                if (abs(x) == 2 || abs(z) == 2)
                {
                    setBlock(level, BlockPos(origin.x + x, (int)mSpike->getHeight(), origin.z + z),
                             Blocks::iron_bars->defaultBlockState(), Block::UPDATE_NONE);
                    setBlock(level, BlockPos(origin.x + x, (int)mSpike->getHeight() + 1, origin.z + z),
                             Blocks::iron_bars->defaultBlockState(), Block::UPDATE_NONE);
                    setBlock(level, BlockPos(origin.x + x, (int)mSpike->getHeight() + 2, origin.z + z),
                             Blocks::iron_bars->defaultBlockState(), Block::UPDATE_NONE);
                }

                setBlock(level, BlockPos(origin.x + x, (int)mSpike->getHeight() + 3, origin.z + z),
                         Blocks::iron_bars->defaultBlockState(), Block::UPDATE_NONE);
            }
        }
    }

    boost::shared_ptr<EnderCrystal> crystal(new EnderCrystal(level));
    if (mCrystalBeamTarget != NULL)
    {
        crystal->setBeamTarget(new BlockPos(*mCrystalBeamTarget));
    }
    else
    {
        crystal->setBeamTarget(NULL);
    }
    crystal->setInvulnerable(mCrystalInvulnerable);
    crystal->moveTo((float)origin.x + 0.5f, (double)((int)mSpike->getHeight() + 1), (float)origin.z + 0.5f,
                    random.nextFloat() * 360.0f, 0.0f);
    level->addEntity(boost::shared_ptr<Entity>(crystal));

    BlockPos bedrockPos(origin.x, (int)mSpike->getHeight(), origin.z);
    setBlock(level, bedrockPos, Blocks::bedrock->defaultBlockState(), Block::UPDATE_NONE);
    delete mSpike;
    mSpike = NULL;
    return true;
}

void SpikeFeature::setCrystalBeamTarget(const BlockPos& pos)
{
    delete mCrystalBeamTarget;
    mCrystalBeamTarget = new BlockPos(pos.x, pos.y, pos.z);
}

SpikeFeature::EndSpike::EndSpike()
    : mCenterX(0),
      mCenterZ(0),
      mRadius(0),
      mHeight(0),
      mGuarded(false),
      mTopBoundingBox(NULL)
{
}

SpikeFeature::EndSpike::EndSpike(const EndSpike& other)
    : mCenterX(other.mCenterX),
      mCenterZ(other.mCenterZ),
      mRadius(other.mRadius),
      mHeight(other.mHeight),
      mGuarded(other.mGuarded),
      mTopBoundingBox(AABB::newPermanent(other.mTopBoundingBox))
{
}

SpikeFeature::EndSpike::EndSpike(int centerX, int centerZ, int radius, int height, bool guarded)
    : mCenterX(centerX),
      mCenterZ(centerZ),
      mRadius(radius),
      mHeight(height),
      mGuarded(guarded),
      mTopBoundingBox(AABB::newPermanent((double)(centerX - radius), 0.0, (double)(centerZ - radius),
                                         (double)(centerX + radius), 256.0, (double)(centerZ + radius)))
{
}

SpikeFeature::EndSpike::~EndSpike()
{
    delete mTopBoundingBox;
}

unsigned int SpikeFeature::EndSpike::getRadius()
{
    return (unsigned int)mRadius;
}

unsigned int SpikeFeature::EndSpike::getHeight()
{
    return (unsigned int)mHeight;
}

unsigned char SpikeFeature::EndSpike::isGuarded()
{
    return (unsigned char)mGuarded;
}

unsigned int SpikeFeature::EndSpike::getCenterX()
{
    return (unsigned int)mCenterX;
}

unsigned int SpikeFeature::EndSpike::getCenterZ()
{
    return (unsigned int)mCenterZ;
}

AABB* SpikeFeature::EndSpike::getTopBoundingBox()
{
    return mTopBoundingBox;
}
