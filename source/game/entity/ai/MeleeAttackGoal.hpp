#ifndef GAME_ENTITY_AI_MELEEATTACKGOAL_HPP
#define GAME_ENTITY_AI_MELEEATTACKGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Level;
class LivingEntity;
class PathfinderMob;
class Path;

class MeleeAttackGoal : public Goal
{
public:
    MeleeAttackGoal(PathfinderMob* mob, double speedModifier, bool followingTargetEvenIfNotSeen);
    virtual ~MeleeAttackGoal();

    void _init(PathfinderMob* mob, double speedModifier, bool followingTargetEvenIfNotSeen);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"MeleeAttack"; }
    virtual void checkAndPerformAttack(boost::shared_ptr<LivingEntity> target, double distance);
    virtual double getAttackReachSqr(boost::shared_ptr<LivingEntity> target);

protected:
    Level* mLevel;
    PathfinderMob* mMob;
    int mAttackInterval;
    int mTicksUntilNextAttack;
    double mSpeedModifier;
    bool mFollowingTargetEvenIfNotSeen;
    Path* mPath;
    int mTicksUntilNextPathRecalculation;
    double mTargetX;
    double mTargetY;
    double mTargetZ;
};

#endif
