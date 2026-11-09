#include "game/entity/ai/MoveToWaterGoal.hpp"

#include "game/core/Mth.hpp"
#include "game/entity/ai/MoveControl.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

MoveToWaterGoal::MoveToWaterGoal(PathfinderMob* mob, double speedModifier, int searchRange, int verticalSearchStart,
                                 double maxStayDistance)
    : MoveToBlockGoal(mob, speedModifier, searchRange, verticalSearchStart, 1.0),
      mSpeedModifierSquared(maxStayDistance * maxStayDistance),
      mEntity(mob)
{
}

MoveToWaterGoal::~MoveToWaterGoal()
{
}

bool MoveToWaterGoal::canUse()
{
    Block* liquid = mEntity->mLevel->getLiquidBlock(BlockPos(mEntity));
    if (liquid->mMaterial == Material::WATER)
        return false;
    return MoveToBlockGoal::canUse();
}

bool MoveToWaterGoal::canContinueToUse()
{
    Block* liquid = mEntity->mLevel->getLiquidBlock(BlockPos(mEntity));
    if (liquid->mMaterial == Material::WATER)
        return false;
    return MoveToBlockGoal::canContinueToUse();
}

void MoveToWaterGoal::stop()
{
    PathNavigation* navigation = mEntity->getNavigation();
    if (navigation != NULL)
        navigation->stop();
    else
        mEntity->getMoveControl()->wait();
}

bool MoveToWaterGoal::isValidTarget(Level*, const BlockPos& pos)
{
    int radius = Mth::floor(mEntity->mBbWidth);
    for (int x = 0; x <= radius; ++x)
    {
        for (int z = 0; z <= radius; ++z)
        {
            BlockPos candidate = pos + BlockPos(x, 0, z);
            Block* liquid = mEntity->mLevel->getLiquidBlock(candidate.above());
            Block* block = mEntity->mLevel->getBlock(candidate.above().above());
            if (liquid->mMaterial != Material::WATER)
                return false;
            if (block->mMaterial != Material::AIR)
                return false;
        }
    }

    mBlockPos = pos;
    return true;
}

void MoveToWaterGoal::tick()
{
    BlockPos above = mBlockPos.above();
    if (mEntity->distanceSqrToBlockPosCenter(above) > mSpeedModifierSquared)
    {
        mReachedTarget = false;
        ++mTicksStayingStill;
        if (mTicksStayingStill % 40 == 0)
        {
            mEntity->getNavigation()->moveTo((double)mBlockPos.x + 0.5,
                                             (double)mBlockPos.y + (double)MoveToBlockGoal::yOff(),
                                             (double)mBlockPos.z + 0.5, mSpeedModifier);
        }
    }
    else
    {
        mReachedTarget = true;
        --mTicksStayingStill;
    }
}

std::wstring MoveToWaterGoal::getName()
{
    return std::wstring(L"Move To Water Goal");
}

int MoveToWaterGoal::GetNextStartTick(PathfinderMob*)
{
    return 20;
}
