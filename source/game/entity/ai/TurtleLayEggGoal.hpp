#ifndef GAME_ENTITY_AI_TURTLELAYEGGGOAL_HPP
#define GAME_ENTITY_AI_TURTLELAYEGGGOAL_HPP

#include <string>

#include "game/world/block/MoveToBlockGoal.hpp"

class Turtle;

class TurtleLayEggGoal : public MoveToBlockGoal
{
public:
    TurtleLayEggGoal(Turtle* turtle, double speedModifier, double searchDistance);
    virtual ~TurtleLayEggGoal();

    bool isNearHome();
    bool homeIsValid();
    virtual bool canUse();
    virtual bool canContinueToUse();
    virtual void tick();
    virtual void start();
    virtual bool isValidTarget(Level* level, const BlockPos& pos);
    virtual int GetNextStartTick(PathfinderMob* mob);
    virtual std::wstring getName() { return L"Turtle Lay Egg"; }

private:
    Turtle* mTurtle;
    int mLayEggCounter;
    double mSearchDistanceSquared;
};

#endif
