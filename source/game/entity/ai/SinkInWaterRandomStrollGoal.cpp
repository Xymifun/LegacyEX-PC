#include "game/entity/ai/SinkInWaterRandomStrollGoal.hpp"

#include "game/entity/ai/GroundPathNavigation.hpp"
#include "game/entity/mob/PathfinderMob.hpp"

SinkInWaterRandomStrollGoal::SinkInWaterRandomStrollGoal(PathfinderMob* mob, double speedModifier)
    : WaterAvoidingRandomStrollGoal(mob, speedModifier)
{
    static_cast<GroundPathNavigation*>(mob->getNavigation())->setSinkInWaterWalk(true);
    static_cast<GroundPathNavigation*>(mob->getNavigation())->setCanFloat(false);
}

SinkInWaterRandomStrollGoal::~SinkInWaterRandomStrollGoal()
{
}

void SinkInWaterRandomStrollGoal::tick()
{
    mMob->isInWater();
    Goal::tick();
}

Vec3* SinkInWaterRandomStrollGoal::getPosition()
{
    if (mMob->isInWater())
    {
        return RandomStrollGoal::getPosition();
    }
    return WaterAvoidingRandomStrollGoal::getPosition();
}
