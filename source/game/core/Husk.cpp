#include "game/core/Husk.hpp"
#include "game/core/EntityType.hpp"

#include <string>

#include "game/client/audio/SoundEvents.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/DifficultyInstance.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"

Husk::Husk(Level* level) : Zombie(level, false)
{
    fjDerivedCtorCalls();
}

Entity* Husk::create(Level* level)
{
    return new Husk(level);
}

void Husk::addDataWalkers(DataFixerUpper* fixer)
{
    Mob::addDataWalkers(fixer, EntityType::HUSK);
}

bool Husk::checkSpawnRules()
{
    return Monster::checkSpawnRules() && mLevel->canSeeSky(BlockPos(this));
}

bool Husk::isSunSensitive() const
{
    return false;
}

const SoundEvent* Husk::getAmbientSound()
{
    return SoundEvents::HUSK_AMBIENT;
}

const SoundEvent* Husk::getHurtSound(const DamageSource*)
{
    return SoundEvents::HUSK_HURT;
}

const SoundEvent* Husk::getDeathSound()
{
    return SoundEvents::HUSK_DEATH;
}

const SoundEvent* Husk::getStepSound()
{
    return SoundEvents::HUSK_STEP;
}

ResourceLocation* Husk::getDefaultLootTable()
{
    return &BuiltInLootTables::HUSK;
}

bool Husk::doHurtTarget(boost::shared_ptr<Entity> target)
{
    bool hurt = Zombie::doHurtTarget(target);
    if (hurt && getMainHandItem()->isEmpty() && eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, target->GetType()))
    {
        float difficulty = mLevel->getCurrentDifficultyAt(BlockPos(this)).getEffectiveDifficulty();
        boost::static_pointer_cast<LivingEntity>(target)->addEffect(
            new MobEffectInstance(MobEffects::HUNGER, 140 * (int)difficulty), boost::shared_ptr<Entity>());
    }
    return hurt;
}

not_null_ptr<ItemInstance> Husk::getSkull()
{
    return ItemInstance::EMPTY;
}

void Husk::finishConversion()
{
    boost::shared_ptr<Zombie> zombie(new Zombie(mLevel));
    zombie->copyPosition(shared_from_this());

    if (isBaby())
    {
        zombie->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(zombie)),
                                 new Zombie::ZombieGroupData(true, isJockey(), false), std::wstring(L""));
    }
    else
    {
        zombie->finalizeMobSpawn(mLevel->getCurrentDifficultyAt(BlockPos(zombie)),
                                 new Zombie::ZombieGroupData(false, false, false), std::wstring(L""));
    }

    playSound(SoundEvents::HUSK_CONVERTED_TO_ZOMBIE, 1.0f, 1.0f);
    mLevel->removeEntity(shared_from_this());
    dropEquipment(true, 0);
    zombie->setNoAi(isNoAi());
    zombie->setCanPickUpLoot(canPickUpLoot());
    zombie->setCanBreakDoors(canBreakDoors());
    mLevel->addEntity(zombie);
}

unsigned long long Husk::GetType()
{
    return EntityType::HUSK;
}
