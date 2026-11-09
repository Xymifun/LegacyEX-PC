#include "game/core/WitherSkull.hpp"
#include "game/core/EntityType.hpp"
#include <algorithm>
#include "game/core/HitResult.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/mob/EntityDamageSource.hpp"
#include "game/core/Difficulty.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/EntityDataSerializers.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

const EntityDataAccessor<bool>* WitherSkull::DATA_DANGEROUS = NULL;

WitherSkull::WitherSkull(Level* level) : Fireball(level)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}
WitherSkull::WitherSkull(Level* level, boost::shared_ptr<LivingEntity> owner, double x, double y, double z)
    : Fireball(level, owner, x, y, z)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}
WitherSkull::WitherSkull(Level* level, double x, double y, double z, double xd, double yd, double zd)
    : Fireball(level, x, y, z, xd, yd, zd)
{
    fjDerivedCtorCalls();
    setSize(0.3125f, 0.3125f);
}
Entity* WitherSkull::create(Level* level)
{
    return new WitherSkull(level);
}
void WitherSkull::staticCtor()
{
    DATA_DANGEROUS = SynchedEntityData::defineId<bool>(EntityType::WITHER_SKULL, EntityType::FIREBALL,
                                                       EntityDataSerializers::BOOLEAN);
}
void WitherSkull::addDataWalkers(DataFixerUpper* fixer)
{
    Fireball::addDataWalkers(fixer, EntityType::WITHER_SKULL);
}
bool WitherSkull::isOnFire()
{
    return false;
}
float WitherSkull::getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos)
{
    float resistance = Entity::getBlockExplosionResistance(explosion, level, pos);
    Block* block = level->getBlock(pos);
    if (isDangerous() && WitherBoss::canDestroy(block))
    {
        const float capped = resistance < 0.8f ? resistance : 0.8f;
        resistance = capped;
    }
    return resistance;
}
bool WitherSkull::isPickable()
{
    return isDangerous();
}
bool WitherSkull::isDangerous()
{
    return mEntityData->get(DATA_DANGEROUS);
}
void WitherSkull::setDangerous(bool dangerous)
{
    mEntityData->set(DATA_DANGEROUS, dangerous, true);
}
float WitherSkull::getInertia()
{
    if (isDangerous())
        return 0.73f;
    return Fireball::getInertia();
}
void WitherSkull::onHit(const HitResult* result)
{
    if (mLevel->mIsClientSide)
        return;
    if (result->mEntity.get() != NULL)
    {
        if (mOwner)
        {
            EntityDamageSource* source = DamageSource::CreateMobAttack(mOwner);
            if (result->mEntity->hurt(source, 8.0f) && result->mEntity->isAlive())
                doEnchantDamageEffects(mOwner, result->mEntity);
            delete source;
        }
        else
        {
            result->mEntity->hurt(DamageSource::MAGIC, 5.0f);
        }
        if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, result->mEntity->GetType()))
        {
            int duration = 0;
            if (mLevel->getDifficulty() == Difficulty::NORMAL)
                duration = 10;
            else if (mLevel->getDifficulty() == Difficulty::HARD)
                duration = 40;
            if (duration != 0)
            {
                boost::dynamic_pointer_cast<LivingEntity>(result->mEntity)
                    ->addEffect(new MobEffectInstance(MobEffects::WITHER, duration * 20, 1),
                                boost::shared_ptr<Entity>());
            }
        }
    }
    mLevel->explode(shared_from_this(), mX, mY, mZ, 1.0f, false,
                    mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING), false, false, false, -1.0f);
    remove();
}
void WitherSkull::defineSynchedData()
{
    mEntityData->createDateItem(DATA_DANGEROUS, false);
}
bool WitherSkull::shouldBurn()
{
    return false;
}
std::vector<unsigned long long>* WitherSkull::getEntityTypesToIgnoreCollisionsWith()
{
    std::vector<unsigned long long>* types = new std::vector<unsigned long long>;
    types->push_back(GetType());
    return types;
}
unsigned long long WitherSkull::GetType()
{
    return EntityType::WITHER_SKULL;
}
