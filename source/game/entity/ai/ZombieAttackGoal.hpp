#ifndef GAME_ENTITY_AI_ZOMBIEATTACKGOAL_HPP
#define GAME_ENTITY_AI_ZOMBIEATTACKGOAL_HPP

#include <string>

#include "game/entity/ai/MeleeAttackGoal.hpp"

class Zombie;

class ZombieAttackGoal : public MeleeAttackGoal
{
public:
    ZombieAttackGoal(Zombie* zombie, double speedModifier, bool followingTargetEvenIfNotSeen);

    virtual void start();
    virtual void stop();
    virtual void tick();
    virtual std::wstring getName() { return L"ZombieAttack"; }

    Zombie* mZombie;
    int mRaiseArmTicks;
};

#endif
