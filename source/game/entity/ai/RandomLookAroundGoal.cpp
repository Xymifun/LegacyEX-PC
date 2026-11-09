#include "game/entity/ai/RandomLookAroundGoal.hpp"

#include <cmath>

#include "game/entity/ai/LookControl.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/util/Random.hpp"

RandomLookAroundGoal::RandomLookAroundGoal(Mob* mob)
{
    mLookAtZ = 0.0;
    mLookAtX = 0.0;
    mLookTime = 0;
    mMob = mob;
    setRequiredControlFlags(3);
}

RandomLookAroundGoal::~RandomLookAroundGoal()
{
}

bool RandomLookAroundGoal::canUse()
{
    return mMob->getRandom()->nextFloat() < 0.02f;
}

bool RandomLookAroundGoal::canContinueToUse()
{
    return mLookTime >= 0;
}

void RandomLookAroundGoal::start()
{
    double angle = mMob->getRandom()->nextDouble() * 6.2831854820251465;
    mLookAtX = std::cos(angle);
    mLookAtZ = std::sin(angle);
    mLookTime = 20 + mMob->getRandom()->nextInt(20);
}

void RandomLookAroundGoal::tick()
{
    --mLookTime;
    mMob->getLookControl()->setLookAt(mMob->mX + mLookAtX, mMob->mY + mMob->getEyeHeight(), mMob->mZ + mLookAtZ,
                                      mMob->getMaxHeadYRot(), mMob->getMaxHeadXRot());
}
