#ifndef GAME_WORLD_GENERATION_VILLAGERGOLEM_HPP
#define GAME_WORLD_GENERATION_VILLAGERGOLEM_HPP

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/Golem.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class Village;
class Entity;
template <class T> class EntityDataAccessor;

bool villagerGolemTargetPredicate(boost::shared_ptr<Entity> entity);

class VillagerGolem : public Golem
{
public:
    VillagerGolem(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();

    virtual void registerGoals();
    virtual void defineSynchedData();
    virtual void customServerAiStep();
    virtual void registerAttributes();
    virtual int decreaseAirSupply(int airSupply);
    virtual void doPush(boost::shared_ptr<Entity> entity);
    virtual void aiStep();
    bool isPlayerCreated();
    bool canAttackType(unsigned long long type);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setPlayerCreated(bool playerCreated);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool doHurtTarget(boost::shared_ptr<Entity> entity);
    virtual void handleEntityEvent(unsigned char event, int data);
    boost::shared_ptr<Village> getVillage();
    unsigned int getAttackAnimationTick();
    void offerFlower(bool offer);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual ResourceLocation* getDefaultLootTable();
    int getOfferFlowerTick();
    virtual void die(const DamageSource* source);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<unsigned char>* DATA_PLAYER_CREATED_ID;
    int mVillageCheckTime;
    boost::weak_ptr<Village> mVillage;
    int mAttackAnimationTick;
    int mOfferFlowerTick;
};

#endif
