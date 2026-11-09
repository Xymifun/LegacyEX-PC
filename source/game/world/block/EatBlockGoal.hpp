#ifndef GAME_WORLD_BLOCK_EATBLOCKGOAL_HPP
#define GAME_WORLD_BLOCK_EATBLOCKGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Level;
class Mob;

class EatBlockGoal : public Goal
{
public:
    EatBlockGoal(Mob* mob);
    virtual ~EatBlockGoal();

    static void staticCtor();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual bool canContinueToUse();
    int getEatAnimationTick();
    virtual void tick();
    virtual void setLevel(Level* level);
    virtual std::wstring getName() { return L"EatBlock"; }

private:
    Mob* mMob;
    Level* mLevel;
    int mEatAnimationTick;
};

#endif
