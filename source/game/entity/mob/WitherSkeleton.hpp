#ifndef GAME_ENTITY_MOB_WITHERSKELETON_HPP
#define GAME_ENTITY_MOB_WITHERSKELETON_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/entity/mob/AbstractSkeleton.hpp"

class DataFixerUpper;
class DamageSource;
class DifficultyInstance;
class Level;
class MobGroupData;
class Arrow;
class WitherBoss;

class WitherSkeleton : public AbstractSkeleton
{
public:
    explicit WitherSkeleton(Level* level);
    virtual ~WitherSkeleton();

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);

    virtual void registerGoals();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual float getEyeHeight();
    virtual bool doHurtTarget(boost::shared_ptr<Entity> target);
    void setWitherParent(const boost::shared_ptr<WitherBoss>& parent);
    boost::shared_ptr<WitherBoss> getWitherParent();
    virtual void die(const DamageSource* source);
    virtual void tick();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual void populateDefaultEquipmentEnchantments(const DifficultyInstance& difficulty);
    boost::shared_ptr<Arrow> getArrow(float distanceFactor);
    virtual unsigned long long GetType();

private:
    boost::weak_ptr<WitherBoss> mWitherParent;
};

#endif
