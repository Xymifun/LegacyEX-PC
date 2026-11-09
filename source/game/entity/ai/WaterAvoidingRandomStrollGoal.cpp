#include "game/entity/ai/WaterAvoidingRandomStrollGoal.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/RandomPos.hpp"

const float WaterAvoidingRandomStrollGoal::PROBABILITY = 0.001f;

WaterAvoidingRandomStrollGoal::WaterAvoidingRandomStrollGoal(PathfinderMob* mob, double speedModifier)
    : RandomStrollGoal(mob, speedModifier),
      mProbability(0.001f)
{
}

WaterAvoidingRandomStrollGoal::WaterAvoidingRandomStrollGoal(PathfinderMob* mob, double speedModifier,
                                                             float probability)
    : RandomStrollGoal(mob, speedModifier),
      mProbability(probability)
{
}

Vec3* WaterAvoidingRandomStrollGoal::getPosition()
{
    boost::shared_ptr<PathfinderMob> mob = boost::static_pointer_cast<PathfinderMob>(mMob->shared_from_this());

    if (mMob->isInWaterOrBubble())
    {
        Vec3* position = RandomPos::getLandPos(mob, 15, 7, -1);
        return position == NULL ? RandomStrollGoal::getPosition() : position;
    }

    if (mMob->getRandom()->nextFloat() >= mProbability)
    {
        return RandomPos::getLandPos(mob, 10, 7, -1);
    }

    return RandomStrollGoal::getPosition();
}
