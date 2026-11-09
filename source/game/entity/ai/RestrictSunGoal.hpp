#ifndef GAME_ENTITY_AI_RESTRICTSUNGOAL_HPP
#define GAME_ENTITY_AI_RESTRICTSUNGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class PathfinderMob;

class RestrictSunGoal : public Goal
{
public:
    RestrictSunGoal(PathfinderMob* mob);
    virtual ~RestrictSunGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual std::wstring getName() { return L"RestrictSun"; }

private:
    PathfinderMob* mMob;
};

#endif
