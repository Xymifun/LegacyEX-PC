#ifndef GAME_ENTITY_MOB_FLYINGMOB_HPP
#define GAME_ENTITY_MOB_FLYINGMOB_HPP

#include "game/entity/mob/Mob.hpp"

class Block;
class BlockPos;
class Level;

class FlyingMob : public Mob
{
public:
    FlyingMob(Level* level);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    virtual void travel(float xxa, float yya, float zza);
    virtual bool onLadder();
};

#endif
