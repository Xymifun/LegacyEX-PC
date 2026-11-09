#ifndef GAME_ENTITY_AI_SWELLGOAL_HPP
#define GAME_ENTITY_AI_SWELLGOAL_HPP

#include <boost/weak_ptr.hpp>
#include <string>

#include "game/entity/ai/Goal.hpp"

class Creeper;
class LivingEntity;

class SwellGoal : public Goal
{
public:
    SwellGoal(Creeper* creeper);

    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"Swell"; }

private:
    Creeper* mCreeper;
    boost::weak_ptr<LivingEntity> mTarget;
};

#endif
