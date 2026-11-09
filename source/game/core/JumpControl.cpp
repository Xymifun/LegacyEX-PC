#include "game/core/JumpControl.hpp"

#include "game/entity/mob/Mob.hpp"

JumpControl::JumpControl(Mob* mob)
{
    mMob = mob;
    mJump = false;
}

void JumpControl::jump()
{
    mJump = true;
}

void JumpControl::tick()
{
    mMob->setJumping(mJump);
    mJump = false;
}
