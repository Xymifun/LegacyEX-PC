#ifndef GAME_ENTITY_AI_FOLLOWMOBGOAL_HPP
#define GAME_ENTITY_AI_FOLLOWMOBGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"
#include "game/world/block/Predicate.hpp"

class AABB;
class Entity;
class Level;
class Mob;
class PathNavigation;
class BlockPathTypes;

class FollowMobGoal : public Goal
{
public:
    class FollowPredicate : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        FollowPredicate(Mob* mob);
        virtual ~FollowPredicate() {}
        virtual bool apply(boost::shared_ptr<Entity> entity) const;

    private:
        Mob* mMob;
    };

    FollowMobGoal(Mob* mob, double speedModifier, float stopDistance, float areaSize);
    virtual ~FollowMobGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"FollowMobGoal"; }

private:
    Mob* mMob;
    FollowPredicate* mPredicate;
    boost::weak_ptr<Mob> mTarget;
    double mSpeedModifier;
    PathNavigation* mNavigation;
    int mTimeToRecalcPath;
    float mStopDistance;
    float mOldWaterCost;
    float mAreaSize;
};

#endif
