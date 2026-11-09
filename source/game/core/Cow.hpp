#ifndef GAME_CORE_COW_HPP
#define GAME_CORE_COW_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Animal.hpp"
#include "game/core/InteractionHand.hpp"

class Block;
class BlockPos;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class SoundEvent;

class Cow : public Animal
{
public:
    Cow(Level* level, bool callDerivedCtor);
    virtual unsigned long long GetType();
    virtual ~Cow() {}

    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void playStepSound(const BlockPos& pos, Block* block);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual float getEyeHeight();
    virtual void registerAttributes();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> parent);
    static Entity* create(Level* level);
};

#endif
