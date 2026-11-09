#ifndef GAME_ENTITY_AI_FOLLOWOWNERGOAL_HPP
#define GAME_ENTITY_AI_FOLLOWOWNERGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Level;
class LivingEntity;
class PathNavigation;
class TamableAnimal;

class FollowOwnerGoal : public Goal
{
public:
    FollowOwnerGoal(TamableAnimal* tamable, double speedModifier, float startDistance, float stopDistance);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"FollowOwner"; }
    virtual bool isTeleportFriendlyBlock(int x, int z, int y, int dx, int dz);

private:
    TamableAnimal* mTamable;
    boost::weak_ptr<LivingEntity> mOwner;

protected:
    Level* mLevel;

private:
    double mSpeedModifier;
    PathNavigation* mNavigation;
    int mTimeToRecalcPath;
    float mStopDistance;
    float mStartDistance;
    float mOldWaterCost;
};

#endif
