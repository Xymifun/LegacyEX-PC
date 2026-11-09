#ifndef GAME_ENTITY_AI_TURTLETRAVELGOAL_HPP
#define GAME_ENTITY_AI_TURTLETRAVELGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Turtle;

class TurtleTravelGoal : public Goal
{
public:
    TurtleTravelGoal(Turtle* turtle, double speedModifier);
    virtual ~TurtleTravelGoal();
    virtual bool canUse();
    virtual void start();
    virtual void tick();
    virtual bool canContinueToUse();
    virtual void stop();
    virtual std::wstring getName() { return L"Turtle Travel"; }

private:
    Turtle* mTurtle;
    double mSpeedModifier;
    bool mReachedPosition;
};

#endif
