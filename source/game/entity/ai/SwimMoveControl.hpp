#ifndef GAME_ENTITY_AI_SWIMMOVECONTROL_HPP
#define GAME_ENTITY_AI_SWIMMOVECONTROL_HPP

#include "game/entity/ai/MoveControl.hpp"

class Mob;

class SwimMoveControl : public MoveControl
{
public:
    explicit SwimMoveControl(Mob* mob);
    virtual ~SwimMoveControl();
    virtual void tick();
};

#endif
