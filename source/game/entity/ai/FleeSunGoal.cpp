#include "game/entity/ai/FleeSunGoal.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/entity/ai/PathNavigation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"

FleeSunGoal::FleeSunGoal(PathfinderMob* mob, double speedModifier)
    : Goal(),
      mMob(mob),
      mWantedX(0.0),
      mWantedY(0.0),
      mWantedZ(0.0),
      mSpeedModifier(speedModifier),
      mLevel(((Entity*)mob)->mLevel)
{
    setRequiredControlFlags(1);
}

FleeSunGoal::~FleeSunGoal()
{
}

Vec3* FleeSunGoal::getHidePos()
{
    Random* random = mMob->getRandom();
    BlockPos origin(mMob->mX, mMob->mBoundingBox->mMinY, mMob->mZ);
    for (int attempt = 0; attempt < 10; ++attempt)
    {
        BlockPos candidate = origin.offset(random->nextInt(20) - 10, random->nextInt(6) - 3, random->nextInt(20) - 10);
        if (!mLevel->canSeeSky(candidate) && mMob->getWalkTargetValue(candidate) < 0.0f)
            return Vec3::newTemp((double)candidate.x, (double)candidate.y, (double)candidate.z);
    }

    return NULL;
}

bool FleeSunGoal::canUse()
{
    if (!mLevel->isDay() || !mMob->isOnFire() || mMob->isUnderLiquid(Material::WATER))
        return false;

    const BlockPos& pos = BlockPos(mMob->mX, mMob->mBoundingBox->mMinY, mMob->mZ);
    if (!mLevel->canSeeSky(pos))
        return false;

    if (!mMob->getItemSlot(EquipmentSlot::HEAD)->isEmpty())
        return false;

    Vec3* hidePos = getHidePos();
    if (hidePos == NULL)
        return false;

    mWantedX = hidePos->x;
    mWantedY = hidePos->y;
    mWantedZ = hidePos->z;
    return true;
}

bool FleeSunGoal::canContinueToUse()
{
    return !mMob->getNavigation()->isDone();
}

void FleeSunGoal::start()
{
    mMob->getNavigation()->moveTo(mWantedX, mWantedY, mWantedZ, mSpeedModifier);
}

void FleeSunGoal::setLevel(Level* level)
{
    mLevel = level;
}
