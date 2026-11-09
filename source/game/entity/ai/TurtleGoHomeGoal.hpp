#ifndef GAME_ENTITY_AI_TURTLEGOHOMEGOAL_HPP
#define GAME_ENTITY_AI_TURTLEGOHOMEGOAL_HPP

#include <string>

#include "game/entity/ai/Goal.hpp"

class Turtle;

class TurtleGoHomeGoal : public Goal
{
public:
    TurtleGoHomeGoal(Turtle* turtle, double speedModifier);
    virtual ~TurtleGoHomeGoal();
    virtual bool canUse();
    virtual void start();
    virtual void stop();
    virtual bool canContinueToUse();
    virtual void tick();
    virtual std::wstring getName() { return L"Turtle Go Home"; }

private:
    Turtle* mTurtle;
    double mSpeedModifier;
    bool mReachedHome;
    int mTravelTime;
};

#endif
