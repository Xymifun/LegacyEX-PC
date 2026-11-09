#include "game/entity/ai/FindUnderwaterTreasureGoal.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/core/AttributeInstance.hpp"
#include "game/entity/mob/SharedMonsterAttributes.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/LargeFeature.hpp"

FindUnderwaterTreasureGoal::FindUnderwaterTreasureGoal(PathfinderMob* mob, float speedModifier, int searchRadius,
                                                       int maxDistance)
    : Goal(),
      mBribed(false),
      mTreasurePos(NULL),
      mTick(0),
      mStartPos(NULL),
      mDistanceTravelled(0),
      mMaxDistance(maxDistance),
      mStuck(0),
      mDirectionChanges(0),
      mSearchRadius(searchRadius),
      mSpeedModifier(speedModifier),
      mMob(mob)
{
}

FindUnderwaterTreasureGoal::~FindUnderwaterTreasureGoal()
{
    delete mTreasurePos;
    delete mStartPos;
}

bool FindUnderwaterTreasureGoal::canUse()
{
    return mBribed && mMob->isInWaterOrBubble() && mMob->getNavigation() != NULL;
}

bool FindUnderwaterTreasureGoal::canContinueToUse()
{
    return canUse() && mTreasurePos != NULL;
}

bool FindUnderwaterTreasureGoal::canInterrupt()
{
    return false;
}

void FindUnderwaterTreasureGoal::start()
{
    Level* level = mMob->mLevel;
    if (!level->mIsClientSide)
    {
        BlockPos pos(mMob->getPos(0.0f));
        BlockPos* ruin = level->findNearestMapFeature(LargeFeature::RUIN, pos, false);
        BlockPos* shipwreck = level->findNearestMapFeature(LargeFeature::SHIPWRECK, pos, false);

        BlockPos* chest = NULL;
        Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(
            BlockPos(Mth::floor(mMob->mX - mSearchRadius), Mth::floor(mMob->mY - 3.0),
                     Mth::floor(mMob->mZ - mSearchRadius)),
            BlockPos(Mth::floor(mMob->mX + mSearchRadius), Mth::floor(mMob->mY + 3.0),
                     Mth::floor(mMob->mZ + mSearchRadius)));
        while (positions->hasNext())
        {
            BlockPos current = *positions->next();
            if (Block::getId(level->getBlock(current)) == BlockId::CHEST)
            {
                chest = new BlockPos(current);
                break;
            }
        }

        if (ruin == NULL)
            mTreasurePos = shipwreck;
        else if (shipwreck == NULL)
            mTreasurePos = ruin;
        else if (pos.distSqr(*shipwreck) < pos.distSqr(*ruin))
        {
            mTreasurePos = shipwreck;
            delete ruin;
        }
        else
        {
            mTreasurePos = ruin;
            delete shipwreck;
        }

        if (mTreasurePos != NULL && chest != NULL
            && pos.distSqr(*mTreasurePos) < (double)(mSearchRadius * mSearchRadius))
        {
            delete mTreasurePos;
            mTreasurePos = chest;
        }
        else
        {
            delete chest;
        }
    }

    mTick = 10;
    mDistanceTravelled = 0;
    mStartPos = new BlockPos(mMob->mX, mMob->mY, mMob->mZ);
    mStuck = 0;
    mDirectionChanges = 0;

    if (mTreasurePos != NULL)
    {
        if (!mMob->getNavigation()->isDone())
            mMob->getNavigation()->stop();
    }
}

void FindUnderwaterTreasureGoal::tick()
{
    if (mTreasurePos == NULL || !mBribed)
        return;

    if (--mTick > 0)
        return;
    mTick = 10;

    if (mMob->distanceToSqr(*mTreasurePos) < 225.0 || mDirectionChanges > 3)
    {
        delete mTreasurePos;
        mBribed = false;
        mTreasurePos = NULL;
        return;
    }

    int distance = (int)std::abs(mMob->distanceTo((double)mStartPos->x, (double)mStartPos->y, (double)mStartPos->z));
    if (distance < 1)
    {
        ++mStuck;
        ++mDirectionChanges;
        mDistanceTravelled += distance;
    }
    else
    {
        mStuck = 0;
        mDistanceTravelled += distance;
    }

    delete mStartPos;
    mStartPos = new BlockPos(mMob->getPos(0.0f));

    if (mMaxDistance < mDistanceTravelled)
    {
        mBribed = false;
        return;
    }

    float xSpread = mMob->getRandom()->nextFloat() * mMob->mBbWidth;
    double particleX = mMob->mX + (xSpread + xSpread) - mMob->mBbWidth;
    float ySpread = mMob->getRandom()->nextFloat() * mMob->mBbHeight;
    double particleY = mMob->mY + 0.5 + ySpread;
    float zSpread = mMob->getRandom()->nextFloat() * mMob->mBbWidth;
    double particleZ = mMob->mZ + (zSpread + zSpread) - mMob->mBbWidth;
    static_cast<ServerLevel*>(mMob->mLevel)
        ->sendParticles(PARTICLE_FIREWORKS_SPARK, particleX, particleY, particleZ, 7, 0.0, 0.0, 0.0, 1.0,
                        arrayWithLength<int>(), 0.0, 0.0, 0.0);

    BlockPos target = mMob->mLevel->getTopSolidBlockPos(*mTreasurePos);
    Vec3* current = mMob->getPos(0.0f);
    Vec3* direction = Vec3::newTemp((double)target.x - current->x, (double)target.y - current->y,
                                    (double)target.z - current->z);

    AttributeInstance* followRange = mMob->getAttribute(SharedMonsterAttributes::FOLLOW_RANGE);
    double range = 16.0;
    if (followRange != NULL)
    {
        range = followRange->getValue();
    }

    if (direction->lengthSqr() > range)
    {
        if (mStuck > 1)
        {
            Direction const* facing = Direction::fromVec3(direction);
            Random* random = mMob->getRandom();
            if (facing == Direction::UP || facing == Direction::DOWN)
            {
                facing = facing->getOpposite();
            }
            else if (random->nextInt(2) == 1)
            {
                facing = facing->getClockWise();
            }
            else
            {
                facing = facing->getCounterClockWise();
            }
            target = BlockPos(current->x + facing->getStepX() * 10, current->y + facing->getStepY() * 10,
                              current->z + facing->getStepZ() * 10);
        }
        else
        {
            target = BlockPos(current->add(direction->normalize()->multiply(range * 0.5)));
        }
    }

    mMob->getNavigation()->moveTo(target.x, target.y, target.z, (double)(mSpeedModifier + 1.0f));
}

void FindUnderwaterTreasureGoal::stop()
{
    delete mTreasurePos;
    mTreasurePos = NULL;
    delete mStartPos;
    mStartPos = NULL;
    mBribed = false;
}

void FindUnderwaterTreasureGoal::setBribed()
{
    mBribed = true;
}

bool FindUnderwaterTreasureGoal::getBribed()
{
    return mBribed;
}

std::wstring FindUnderwaterTreasureGoal::getName()
{
    return std::wstring(L"Find Treasure");
}
