#ifndef GAME_ENTITY_AI_WITHERRANDOMATTACKPOSGOAL_HPP
#define GAME_ENTITY_AI_WITHERRANDOMATTACKPOSGOAL_HPP

#include "game/entity/ai/RandomStrollGoal.hpp"

class PathfinderMob;

class WitherRandomAttackPosGoal : public RandomStrollGoal
{
public:
    WitherRandomAttackPosGoal(PathfinderMob* mob, float speedModifier, int interval, int xz, int y);
    virtual ~WitherRandomAttackPosGoal();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void start();
    virtual void stop();

private:
    bool mIsPathing;
    int mXz;
    int mY;
};

#endif
