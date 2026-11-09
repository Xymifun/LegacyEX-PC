#include "game/entity/ai/TurtleGoToWaterGoal.hpp"

#include "game/core/Turtle.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

TurtleGoToWaterGoal::TurtleGoToWaterGoal(Turtle* turtle, double speedModifier)
    : MoveToBlockGoal(turtle, speedModifier, 16, 1, 1.0)
{
    if (turtle->isBaby())
    {
        mSpeedModifier = 2.0;
    }

    mTurtle = turtle;
}

TurtleGoToWaterGoal::~TurtleGoToWaterGoal()
{
}

bool TurtleGoToWaterGoal::canUse()
{
    if ((mTurtle->isBaby() && !mTurtle->isInWater()) || (!mTurtle->isGoingHome() && !mTurtle->isInWater()))
    {
        return MoveToBlockGoal::canUse();
    }

    return false;
}

bool TurtleGoToWaterGoal::canContinueToUse()
{
    return !mTurtle->isInWater() && mTicksStayingStill <= 1200 && isValidTarget(mTurtle->mLevel, mBlockPos);
}

bool TurtleGoToWaterGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlockState(pos)->getBlock();
    return block == Blocks::water || block == Blocks::flowing_water;
}
