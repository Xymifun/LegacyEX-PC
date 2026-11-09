#ifndef GAME_ENTITY_AI_TURTLEPANICGOAL_HPP
#define GAME_ENTITY_AI_TURTLEPANICGOAL_HPP

#include <string>

#include "game/entity/ai/PanicGoal.hpp"

class Turtle;

class TurtlePanicGoal : public PanicGoal
{
public:
    TurtlePanicGoal(Turtle* turtle, double speedModifier);
    virtual ~TurtlePanicGoal();

    virtual bool canUse();
    virtual std::wstring getName() { return L"Turtle Panic"; }
};

#endif
