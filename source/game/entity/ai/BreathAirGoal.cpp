#include "game/entity/ai/BreathAirGoal.hpp"

#include "game/core/Dolphin.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/Level.hpp"

BreathAirGoal::BreathAirGoal(Dolphin* dolphin) : Goal(), mAirSupplyThreshold(100), mDolphin(dolphin)
{
}

BreathAirGoal::~BreathAirGoal()
{
}

BlockPos* BreathAirGoal::findAirInArea(const BlockPos& minPos, const BlockPos& maxPos)
{
    for (int x = minPos.x; x <= maxPos.x; ++x)
    {
        for (int z = minPos.z; z <= maxPos.z; ++z)
        {
            for (int y = minPos.y; y <= maxPos.y; ++y)
            {
                if (mDolphin->mLevel->getBlockState(BlockPos(x, y, z))->getMaterial() == Material::AIR)
                    return new BlockPos(x, y, z);
            }
        }
    }

    return NULL;
}

void BreathAirGoal::findAirPosition()
{
    const double radius = 2.0;
    const double verticalRange = 16.0;
    BlockPos minPos(Mth::fastFloor(mDolphin->mX - radius), Mth::fastFloor(mDolphin->mY),
                    Mth::fastFloor(mDolphin->mZ - radius));
    BlockPos maxPos(Mth::fastFloor(mDolphin->mX + radius), Mth::fastFloor(mDolphin->mY + verticalRange),
                    Mth::fastFloor(mDolphin->mZ + radius));
    BlockPos* pos = findAirInArea(minPos, maxPos);
    if (pos == NULL)
        pos = new BlockPos(mDolphin->mX, mDolphin->mY + verticalRange, mDolphin->mZ);

    mDolphin->getNavigation()->moveTo(pos->x, pos->y - 1, pos->z, 1.0);
    delete pos;
}

void BreathAirGoal::start()
{
    findAirPosition();
}

void BreathAirGoal::tick()
{
    if (mDolphin->getNavigation()->isDone())
    {
        Material* material = mDolphin->mLevel->getBlockState(BlockPos(mDolphin))->getMaterial();
        if (material != Material::AIR)
            findAirPosition();
    }

    mDolphin->moveRelative(mDolphin->mXxa, mDolphin->mYya, mDolphin->mZza, 0.02f);
    mDolphin->move(eMoverType_Self, mDolphin->mXd, mDolphin->mYd, mDolphin->mZd, false);
}

bool BreathAirGoal::canUse()
{
    return mDolphin->getAirSupply() < mAirSupplyThreshold;
}

bool BreathAirGoal::canContinueToUse()
{
    return canUse();
}

bool BreathAirGoal::canInterrupt()
{
    return false;
}

std::wstring BreathAirGoal::getName()
{
    return std::wstring(L"Breath Air");
}
