#ifndef GAME_CORE_JUMPCONTROL_HPP
#define GAME_CORE_JUMPCONTROL_HPP

#include "game/entity/ai/Control.hpp"

class Mob;

class JumpControl : public Control
{
public:
    explicit JumpControl(Mob* mob);

    virtual ~JumpControl() {}
    virtual void tick();

    void jump();

protected:
    Mob* mMob;
    bool mJump;
};

#endif
