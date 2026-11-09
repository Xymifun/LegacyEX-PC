#ifndef GAME_CORE_SNOWMAN_HPP
#define GAME_CORE_SNOWMAN_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/Golem.hpp"
#include "game/entity/mob/RangedAttackMob.hpp"
#include "game/core/InteractionHand.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class LivingEntity;
class SoundEvent;
template <class T> class EntityDataAccessor;

class SnowMan : public Golem, public RangedAttackMob
{
public:
    SnowMan(Level* level);

    static Entity* create(Level* level);
    virtual void performRangedAttack(boost::shared_ptr<LivingEntity> target, float distanceFactor);
    virtual bool isAggressive();
    virtual void setAggressive(bool aggressive);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    bool hasPumpkin();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setPumpkin(bool pumpkin);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void aiStep();
    virtual ResourceLocation* getDefaultLootTable();
    virtual float getEyeHeight();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<unsigned char>* DATA_PUMPKIN;
};

#endif
