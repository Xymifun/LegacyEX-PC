#ifndef GAME_WORLD_GENERATION_ZOMBIEVILLAGER_HPP
#define GAME_WORLD_GENERATION_ZOMBIEVILLAGER_HPP

#include <string>

#include "game/entity/mob/Zombie.hpp"

class CompoundTag;
class DataFixerUpper;
class DifficultyInstance;
class EntityTooltipDataHolder;
class Player;
class SoundEvent;

template <class T> class EntityDataAccessor;

class ZombieVillager : public Zombie
{
public:
    ZombieVillager(Level* level);

    static Entity* create(Level* level);
    static void staticCtor();
    virtual void defineSynchedData();
    void setProfession(int profession);
    int getProfession();
    static void addDataWalkers(DataFixerUpper* fixer);
    bool isConverting();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void startConverting(const std::wstring& player, int conversionTime);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    int getConversionProgress();
    virtual void tick();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual bool removeWhenFarAway();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void finishConversion();
    virtual float getVoicePitch();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getStepSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual ResourceLocation* getDefaultGearTable();
    virtual not_null_ptr<ItemInstance> getSkull();
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<bool>* DATA_CONVERTING_ID;
    static const EntityDataAccessor<int>* DATA_PROFESSION_ID;

    int mConversionTime;
    std::wstring mConversionPlayer;
};

#endif
