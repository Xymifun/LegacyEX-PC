#ifndef GAME_ENTITY_MOB_PIGZOMBIE_HPP
#define GAME_ENTITY_MOB_PIGZOMBIE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/HurtByTargetGoal.hpp"
#include "game/entity/mob/Zombie.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class DifficultyInstance;
class Level;
class PathfinderMob;
class Player;
class SoundEvent;
class Entity;

class PigZombie : public Zombie
{
public:
    class PigZombieHurtByOtherGoal : public HurtByTargetGoal
    {
    public:
        PigZombieHurtByOtherGoal(PigZombie* mob);
        virtual void alertOther(boost::shared_ptr<PathfinderMob> mob, boost::shared_ptr<LivingEntity> target);
    };

    static AttributeModifier* SPEED_MODIFIER_ATTACKING;

    PigZombie(Level* level);

    static Entity* create(Level* level);
    void _init();
    virtual void setLastHurtByMob(boost::shared_ptr<LivingEntity> entity);
    virtual void addBehaviourGoals();
    virtual void registerAttributes();
    bool isAngry();
    virtual void customServerAiStep();
    virtual bool checkSpawnRules();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    void makeAngry(boost::shared_ptr<Entity> entity);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void populateDefaultEquipmentSlots(const DifficultyInstance& difficulty);
    virtual not_null_ptr<ItemInstance> getSkull();
    virtual bool isPreventingPlayerRest(boost::shared_ptr<Player> player);
    virtual unsigned long long GetType();

private:
    int mRemainingPersistentAngerTime;
    int mAngerSoundDelay;
};

#endif
