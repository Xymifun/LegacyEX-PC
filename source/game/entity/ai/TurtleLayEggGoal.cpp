#include "game/entity/ai/TurtleLayEggGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/TurtleEggBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

TurtleLayEggGoal::TurtleLayEggGoal(Turtle* turtle, double speedModifier, double searchDistance)
    : MoveToBlockGoal(turtle, speedModifier, Mth::ceil((float)searchDistance), 1, 1.0)
{
    mLayEggCounter = 0;
    mTurtle = turtle;
    mSearchDistanceSquared = searchDistance * searchDistance;
}

TurtleLayEggGoal::~TurtleLayEggGoal()
{
}

bool TurtleLayEggGoal::isNearHome()
{
    BlockPos position(mTurtle->getPos(0.0f));
    BlockPos home = mTurtle->getHomePos();
    return position.distSqr((double)home.x, (double)home.y, (double)home.z) < mSearchDistanceSquared;
}

bool TurtleLayEggGoal::homeIsValid()
{
    int verticalSearchStart = mVerticalSearchStart;
    int searchRange = mSearchRange;
    for (int y = 0; y <= verticalSearchStart; y = y > 0 ? -y : 1 - y)
    {
        for (int radius = 0; radius < searchRange; ++radius)
        {
            for (int x = 0; x <= radius; x = x > 0 ? -x : 1 - x)
            {
                for (int z = x < radius && x > -radius ? radius : 0; z <= radius; z = z > 0 ? -z : 1 - z)
                {
                    BlockPos candidate = mTurtle->getHomePos().offset(x, y - 1, z);
                    if (isValidTarget(mTurtle->mLevel, candidate))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool TurtleLayEggGoal::canUse()
{
    if (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg() || !isNearHome())
        return false;

    if (MoveToBlockGoal::canUse())
        return true;

    if (mNextStartTick == GetNextStartTick(mTurtle) && mBlockPos == BlockPos::ZERO && !homeIsValid())
        mTurtle->GiveUpOnEgg();

    return false;
}

bool TurtleLayEggGoal::canContinueToUse()
{
    if (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg() || !isNearHome())
        return false;

    if (mLayEggCounter < 1)
        return MoveToBlockGoal::canContinueToUse();

    return true;
}

void TurtleLayEggGoal::tick()
{
    MoveToBlockGoal::tick();

    Level* level = mTurtle->mLevel;
    BlockPos position(mTurtle->getPos(0.0f));
    if (!isNearHome())
        return;

    if (!isReachedTarget() && !isValidTarget(level, position.below()))
        return;

    if (mLayEggCounter < 1)
    {
        mTurtle->setIsLayingEgg(true);
    }
    else if (mLayEggCounter > 200)
    {
        int eggs = level->mRandom->nextInt(4);
        level->playSound(boost::shared_ptr<Player>(), position, SoundEvents::TURTLE_LAY_EGG,
                         SoundSource::eSoundSource_Blocks, 0.3f, level->mRandom->nextFloat() * 0.2f + 0.9f, 16.0f);
        level->setBlock(position, Blocks::turtle_egg->defaultBlockState()->setValue(TurtleEggBlock::EGG_COUNT, eggs),
                        Block::UPDATE_ALL, false);
        mTurtle->setHasEgg(false);
        mTurtle->setIsLayingEgg(false);
    }

    if (mTurtle->isLayingEgg())
        ++mLayEggCounter;
}

void TurtleLayEggGoal::start()
{
    MoveToBlockGoal::start();
    mLayEggCounter = 0;
}

bool TurtleLayEggGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    int radius = Mth::floor(mTurtle->mBbWidth);
    int originX = pos.x;
    int originY = pos.y;
    int originZ = pos.z;
    for (int x = -radius; x <= radius; ++x)
    {
        for (int z = -radius; z <= radius; ++z)
        {
            BlockPos candidate(originX + x, originY, originZ + z);
            if (!level->isEmptyBlock(candidate.above()) || level->getBlock(candidate) != Blocks::sand)
                return false;
        }
    }

    return true;
}

int TurtleLayEggGoal::GetNextStartTick(PathfinderMob*)
{
    return 20;
}
