#ifndef GAME_ENTITY_AI_MOVETOWARDSTARGETGOAL_HPP
#define GAME_ENTITY_AI_MOVETOWARDSTARGETGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/util/Vec3.hpp"

class LivingEntity;
class PathfinderMob;

class MoveTowardsTargetGoal : public Goal
{
public:
    MoveTowardsTargetGoal(PathfinderMob* mob, double speedModifier, float within);

    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"MoveTowardsTarget"; }

private:
    PathfinderMob* mMob;
    boost::weak_ptr<LivingEntity> mTarget;
    double mWantedX;
    double mWantedY;
    double mWantedZ;
    double mSpeedModifier;
    float mWithin;
};

#endif
