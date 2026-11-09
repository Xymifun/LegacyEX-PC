#ifndef GAME_ENTITY_MOB_PLAYERRIDEABLEJUMPING_HPP
#define GAME_ENTITY_MOB_PLAYERRIDEABLEJUMPING_HPP

#include "game/entity/mob/PlayerRideable.hpp"

class PlayerRideableJumping : public PlayerRideable
{
public:
    virtual void onPlayerJump(int jump) = 0;
    virtual bool canJump() = 0;
    virtual double getJumpRidingScale() = 0;
    virtual void setJumpRidingScale(float scale) = 0;
    virtual void handleStartJump(int jump) = 0;
    virtual void handleStopJump() = 0;
};

#endif
