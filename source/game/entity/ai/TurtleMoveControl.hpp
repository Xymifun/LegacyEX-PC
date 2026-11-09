#ifndef GAME_ENTITY_AI_TURTLEMOVECONTROL_HPP
#define GAME_ENTITY_AI_TURTLEMOVECONTROL_HPP

#include "game/entity/ai/MoveControl.hpp"

class Turtle;

class TurtleMoveControl : public MoveControl
{
public:
    explicit TurtleMoveControl(Turtle* turtle);
    virtual ~TurtleMoveControl();

    virtual void tick();

private:
    Turtle* mTurtle;

    void updateSpeed();
};

#endif
