#ifndef GAME_ENTITY_AI_DOLPHINLOOKCONTROL_HPP
#define GAME_ENTITY_AI_DOLPHINLOOKCONTROL_HPP

#include "game/entity/ai/LookControl.hpp"

class Mob;

class DolphinLookControl : public LookControl
{
public:
    DolphinLookControl(Mob* mob, int maxYRotFromCenter);
    virtual ~DolphinLookControl();
    virtual void tick();

private:
    int mMaxYRotFromCenter;
};

#endif
