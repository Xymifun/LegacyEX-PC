#ifndef GAME_ENTITY_AI_RANDOMLOOKAROUNDGOAL_HPP
#define GAME_ENTITY_AI_RANDOMLOOKAROUNDGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Mob;

class RandomLookAroundGoal : public Goal
{
public:
    RandomLookAroundGoal(Mob* mob);
    virtual ~RandomLookAroundGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void tick();
    virtual std::wstring getName() { return L"RandomLookAround"; }

protected:
    Mob* mMob;
    double mLookAtX;
    double mLookAtZ;
    int mLookTime;
};

#endif
