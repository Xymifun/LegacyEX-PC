#ifndef GAME_ENTITY_AI_SKELETONTRAPGOAL_HPP
#define GAME_ENTITY_AI_SKELETONTRAPGOAL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/Goal.hpp"

class AbstractHorse;
class DifficultyInstance;
class Skeleton;
class SkeletonHorse;

class SkeletonTrapGoal : public Goal
{
public:
    SkeletonTrapGoal(SkeletonHorse* horse);
    virtual ~SkeletonTrapGoal() {}
    virtual bool canUse();
    virtual void tick();
    virtual std::wstring getName() { return L"SkeletonTrap"; }

private:
    boost::shared_ptr<Skeleton> createSkeleton(const DifficultyInstance& difficulty,
                                               const boost::shared_ptr<AbstractHorse>& horse);
    boost::shared_ptr<AbstractHorse> createHorse(const DifficultyInstance& difficulty);

    SkeletonHorse* mHorse;
};

#endif
