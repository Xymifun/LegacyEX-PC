#ifndef GAME_ENTITY_AI_AVOIDENTITYGOAL_HPP
#define GAME_ENTITY_AI_AVOIDENTITYGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <typeinfo>

#include "game/entity/ai/Goal.hpp"

class AvoidEntityGoalEntitySelector;
class Entity;
class PathfinderMob;
class Path;
class PathNavigation;

template <class T> class Predicate;

class AvoidEntityGoal : public Goal
{
    friend class AvoidEntityGoalEntitySelector;

public:
    AvoidEntityGoal(PathfinderMob* mob, const std::type_info& avoidType, float maxDistance, double walkSpeedModifier,
                    double sprintSpeedModifier);
    AvoidEntityGoal(PathfinderMob* mob, const std::type_info& avoidType,
                    const Predicate<boost::shared_ptr<Entity> >* selector, float maxDistance, double walkSpeedModifier,
                    double sprintSpeedModifier);
    virtual ~AvoidEntityGoal();

    void _init(PathfinderMob* mob, const Predicate<boost::shared_ptr<Entity> >* selector, float maxDistance,
               double walkSpeedModifier, double sprintSpeedModifier);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"AvoidEntity"; }

private:
    PathfinderMob* mMob;
    double mWalkSpeedModifier;
    double mSprintSpeedModifier;
    float mMaxDistance;
    Path* mPath;
    PathNavigation* mNavigation;
    const std::type_info* mAvoidType;
    const Predicate<boost::shared_ptr<Entity> >* mSelector;
    AvoidEntityGoalEntitySelector* mEntitySelector;

protected:
    boost::weak_ptr<Entity> mToAvoid;
};

#endif
