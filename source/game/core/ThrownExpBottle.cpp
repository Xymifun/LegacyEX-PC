#include "game/core/ThrownExpBottle.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/ExperienceOrb.hpp"
#include "game/core/HitResult.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

ThrownExpBottle::ThrownExpBottle(Level* level) : ThrowableProjectile(level)
{
    _init();
}

ThrownExpBottle::ThrownExpBottle(Level* level, boost::shared_ptr<LivingEntity> owner)
    : ThrowableProjectile(level, owner)
{
    _init();
}

ThrownExpBottle::ThrownExpBottle(Level* level, double x, double y, double z) : ThrowableProjectile(level, x, y, z)
{
    _init();
}

Entity* ThrownExpBottle::create(Level* level)
{
    return new ThrownExpBottle(level);
}

bool ThrownExpBottle::onHit(const HitResult* result)
{
    Projectile::onHit(result);
    if (!mLevel->mIsClientSide)
    {
        mLevel->levelEvent(LevelEvent::PARTICLES_SPELL_POTION_SPLASH, BlockPos(shared_from_this()),
                           PotionUtils::getColor(Potions::WATER));
        int experience = 3 + mLevel->mRandom->nextInt(5) + mLevel->mRandom->nextInt(5);
        while (experience > 0)
        {
            int value = ExperienceOrb::getExperienceValue(experience);
            experience -= value;
            mLevel->addEntity(boost::shared_ptr<ExperienceOrb>(new ExperienceOrb(mLevel, mX, mY, mZ, value)));
        }
        remove();
    }
    return true;
}

void ThrownExpBottle::addDataWalkers(DataFixerUpper*)
{
}

void ThrownExpBottle::_init()
{
    fjDerivedCtorCalls();
}

float ThrownExpBottle::getGravity()
{
    return 0.07f;
}

unsigned long long ThrownExpBottle::GetType()
{
    return EntityType::THROWN_EXP_BOTTLE;
}
