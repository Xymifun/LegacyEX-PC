#ifndef GAME_ENTITY_MOB_ZOMBIEHORSE_HPP
#define GAME_ENTITY_MOB_ZOMBIEHORSE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/InteractionHand.hpp"

class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;

class ZombieHorse : public AbstractHorse
{
public:
    ZombieHorse(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void registerAttributes();
    virtual void registerGoals();
    virtual EMobType getMobType();
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool breathesWater();
    virtual unsigned long long GetType();
};

#endif
