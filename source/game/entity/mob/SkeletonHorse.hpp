#ifndef GAME_ENTITY_MOB_SKELETONHORSE_HPP
#define GAME_ENTITY_MOB_SKELETONHORSE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/ai/SkeletonTrapGoal.hpp"
#include "game/entity/mob/AbstractHorse.hpp"

class Block;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class SoundEvent;
class SoundType;

class SkeletonHorse : public AbstractHorse
{
public:
    SkeletonHorse(Level* level);

    static Entity* create(Level* level);
    void setTrap(bool trap);
    virtual void registerAttributes();
    virtual void registerGoals();
    virtual void playUnderwaterSound(const BlockPos& pos, Block* block);
    virtual void playGallopSound(const SoundType* sound);
    virtual const SoundEvent* getJumpSound();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual EMobType getMobType();
    virtual double getRideHeight();
    virtual ResourceLocation* getDefaultLootTable();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool isTamed();
    bool isTrap();
    virtual void aiStep();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool breathesWater();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual unsigned long long GetType();

private:
    SkeletonTrapGoal mSkeletonTrapGoal;
    bool mTrap;
    int mTrapTime;
};

#endif
