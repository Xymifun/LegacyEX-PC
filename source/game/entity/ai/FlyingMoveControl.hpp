#ifndef GAME_ENTITY_AI_FLYINGMOVECONTROL_HPP
#define GAME_ENTITY_AI_FLYINGMOVECONTROL_HPP

#include "game/entity/ai/MoveControl.hpp"

class Mob;

class FlyingMoveControl : public MoveControl
{
public:
    explicit FlyingMoveControl(Mob* mob);
    virtual ~FlyingMoveControl();

    virtual void tick();
};

#endif
