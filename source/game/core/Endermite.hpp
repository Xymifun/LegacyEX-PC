#ifndef GAME_CORE_ENDERMITE_HPP
#define GAME_CORE_ENDERMITE_HPP

#include "game/entity/mob/Monster.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Level;
class SoundEvent;

class Endermite : public Monster
{
public:
    Endermite(Level* level);
    virtual unsigned long long GetType();
    virtual void tick();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual bool makeStepSound();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual double getRidingHeight();
    virtual void setYBodyRot(float yBodyRot);
    virtual float getEyeHeight();
    virtual void registerAttributes();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual EMobType getMobType();
    virtual void aiStep();
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool checkSpawnRules();
    virtual bool isDarkEnoughToSpawn();

    static void addDataWalkers(DataFixerUpper* fixer);
    static Entity* create(Level* level);
    bool isPlayerSpawned();
    void setPlayerSpawned(bool playerSpawned);

private:
    int mLifetime;
    bool mPlayerSpawned;
};

#endif
