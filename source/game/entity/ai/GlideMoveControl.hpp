#ifndef GAME_ENTITY_AI_GLIDEMOVECONTROL_HPP
#define GAME_ENTITY_AI_GLIDEMOVECONTROL_HPP

#include "game/entity/ai/MoveControl.hpp"

class Mob;

class GlideMoveControl : public MoveControl
{
public:
    GlideMoveControl(Mob* mob, float startSpeed, float speedWhenTurning);
    virtual ~GlideMoveControl();
    virtual void tick();

private:
    float mStartSpeed;
    float mSpeedWhenTurning;
    float mSpeed;
};

#endif
