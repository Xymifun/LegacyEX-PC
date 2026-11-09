#ifndef GAME_ENTITY_AI_TURTLEBREEDGOAL_HPP
#define GAME_ENTITY_AI_TURTLEBREEDGOAL_HPP

#include <string>

#include "game/entity/ai/BreedGoal.hpp"

class Turtle;

class TurtleBreedGoal : public BreedGoal
{
public:
    TurtleBreedGoal(Turtle* turtle, double speedModifier);
    virtual bool canUse();
    virtual std::wstring getName() { return L"Turtle Breed"; }
    virtual void breed();

private:
    Turtle* mTurtle;
};

#endif
