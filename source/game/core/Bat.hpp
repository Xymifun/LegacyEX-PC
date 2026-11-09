#ifndef GAME_CORE_BAT_HPP
#define GAME_CORE_BAT_HPP

#include "game/core/AmbientCreature.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;

template <class T> class EntityDataAccessor;

class Bat : public AmbientCreature
{
public:
    Bat(Level* level);
    virtual unsigned long long GetType();
    virtual ~Bat();
    virtual void defineSynchedData();
    virtual void tick();
    virtual bool makeStepSound();
    virtual void checkFallDamage(double y, bool onGround, Block* block, const BlockPos& pos);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual bool isIgnoringBlockTriggers();
    virtual float getEyeHeight();
    virtual void registerAttributes();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual float getVoicePitch();
    virtual void pushEntities();
    virtual void doPush(boost::shared_ptr<Entity> entity);
    virtual const SoundEvent* getAmbientSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual void customServerAiStep();
    virtual bool checkSpawnRules();
    virtual bool isResting();
    virtual void setResting(bool resting);

    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    static Entity* create(Level* level);

private:
    static const EntityDataAccessor<unsigned char>* DATA_ID_FLAGS;
    BlockPos* mTargetPosition;
};

#endif
