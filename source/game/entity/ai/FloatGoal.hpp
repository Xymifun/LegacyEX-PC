#ifndef GAME_ENTITY_AI_FLOATGOAL_HPP
#define GAME_ENTITY_AI_FLOATGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Mob;

class FloatGoal : public Goal
{
public:
    FloatGoal(Mob* mob);
    virtual ~FloatGoal();
    virtual bool canUse();
    virtual void tick();
    virtual std::wstring getName() { return L"Float"; }

private:
    Mob* mMob;
};

#endif
