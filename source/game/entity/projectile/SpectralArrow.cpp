#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"

SpectralArrow::SpectralArrow(Level* level) : Arrow(level)
{
    fjDerivedCtorCalls();
    init();
}

SpectralArrow::SpectralArrow(Level* level, boost::shared_ptr<LivingEntity> owner) : Arrow(level, owner)
{
    fjDerivedCtorCalls();
    init();
}

SpectralArrow::SpectralArrow(Level* level, double x, double y, double z) : Arrow(level, x, y, z)
{
    fjDerivedCtorCalls();
    init();
}

Entity* SpectralArrow::create(Level* level)
{
    return new SpectralArrow(level);
}

void SpectralArrow::init()
{
    mDuration = 0;
    defineSynchedData();
}

void SpectralArrow::tick()
{
    AbstractRotatedItemProjectile::tick();
    if (mLevel->mIsClientSide && !mInGround)
        mLevel->addParticle(PARTICLE_INSTANT_SPELL, mX, mY, mZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

void SpectralArrow::addDataWalkers(DataFixerUpper* fixer)
{
    Arrow::addDataWalkers(fixer, EntityType::SPECTRAL_ARROW);
}

void SpectralArrow::readAdditionalSaveData(CompoundTag* tag)
{
    Arrow::readAdditionalSaveData(tag);
    if (tag->contains(L"Duration"))
        mDuration = tag->getInt(L"Duration");
}

void SpectralArrow::addAdditonalSaveData(CompoundTag* tag)
{
    Arrow::addAdditonalSaveData(tag);
    tag->putInt(L"Duration", mDuration);
}

not_null_ptr<ItemInstance> SpectralArrow::getPickupItem()
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Items::spectralArrow));
}

void SpectralArrow::doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity)
{
    AbstractRotatedItemProjectile::doPostHurtEffects(entity);
    MobEffectInstance* effect = new MobEffectInstance(MobEffects::GLOWING, mDuration, 0);
    entity->addEffect(effect, boost::shared_ptr<Entity>());
}

unsigned long long SpectralArrow::GetType()
{
    return EntityType::SPECTRAL_ARROW;
}
