#ifndef GAME_ENTITY_AI_TURTLERANDOMSTROLLGOAL_HPP
#define GAME_ENTITY_AI_TURTLERANDOMSTROLLGOAL_HPP

#include "game/entity/ai/RandomStrollGoal.hpp"

class Turtle;

class TurtleRandomStrollGoal : public RandomStrollGoal
{
public:
    TurtleRandomStrollGoal(Turtle* turtle, double speedModifier, int interval);
    virtual ~TurtleRandomStrollGoal();
    virtual bool canUse();
    virtual std::wstring getName() { return L"Turtle Random Stroll"; }
};

#endif
