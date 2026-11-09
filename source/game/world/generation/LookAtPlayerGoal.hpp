#ifndef GAME_WORLD_GENERATION_LOOKATPLAYERGOAL_HPP
#define GAME_WORLD_GENERATION_LOOKATPLAYERGOAL_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <string>
#include <typeinfo>

#include "game/entity/ai/Goal.hpp"
#include "game/world/block/Predicate.hpp"

class Entity;
class Mob;

class LookAtPlayerGoal : public Goal
{
public:
    class LookAtPlayerGoalEntitySelector : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        virtual bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~LookAtPlayerGoalEntitySelector();
    };

    LookAtPlayerGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance);
    LookAtPlayerGoal(Mob* mob, const std::type_info& lookAtType, float lookDistance, float probability);
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"LookAtPlayer"; }

private:
    Mob* mMob;

protected:
    boost::weak_ptr<Entity> mLookAt;

private:
    float mLookDistance;
    int mLookTime;
    float mProbability;
    const std::type_info* mLookAtType;
};

#endif
