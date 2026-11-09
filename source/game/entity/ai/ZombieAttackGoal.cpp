#include "game/entity/ai/ZombieAttackGoal.hpp"

#include "game/entity/mob/Zombie.hpp"

void ZombieAttackGoal::start()
{
    MeleeAttackGoal::start();
    mRaiseArmTicks = 0;
}

void ZombieAttackGoal::stop()
{
    MeleeAttackGoal::stop();
    mZombie->setAggressive(false);
}

void ZombieAttackGoal::tick()
{
    MeleeAttackGoal::tick();

    ++mRaiseArmTicks;

    if (mRaiseArmTicks >= 5 && mTicksUntilNextAttack < mAttackInterval / 2)
    {
        mZombie->setAggressive(true);
    }
    else
    {
        mZombie->setAggressive(false);
    }
}
