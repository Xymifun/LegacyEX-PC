#ifndef GAME_ENTITY_AI_TARGETGOAL_HPP
#define GAME_ENTITY_AI_TARGETGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/ai/Goal.hpp"

class LivingEntity;
class Mob;
class PathfinderMob;

class TargetGoal : public Goal
{
public:
    TargetGoal(PathfinderMob* mob, bool mustSee);
    TargetGoal(PathfinderMob* mob, bool mustSee, bool mustReach);

    void _init(PathfinderMob* mob, bool mustSee, bool mustReach);
    virtual bool canContinueToUse();
    virtual double getFollowDistance();
    virtual void start();
    virtual void stop();
    static bool canAttack(Mob* mob, boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers,
                          bool requireSight);
    bool canReach(boost::shared_ptr<LivingEntity> target);
    virtual bool canAttack(boost::shared_ptr<LivingEntity> target, bool allowInvulnerablePlayers);
    TargetGoal* setUnseenMemoryTicks(int ticks);
    virtual std::wstring getName() { return std::wstring(L"TargetGoal"); }

protected:
    PathfinderMob* mMob;
    bool mMustSee;
    bool mMustReach;
    int mCanReach;
    int mTargetSearchDelay;
    int mUnseenTicks;
    boost::weak_ptr<LivingEntity> mTarget;
    int mUnseenMemoryTicks;
};

#endif
