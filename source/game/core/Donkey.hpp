#ifndef GAME_CORE_DONKEY_HPP
#define GAME_CORE_DONKEY_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractChestedHorse.hpp"

class AgableMob;
class Animal;
class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class SoundEvent;

class Donkey : public AbstractChestedHorse
{
public:
    Donkey(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual ResourceLocation* getDefaultLootTable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool HideUseTooltips();
    virtual unsigned long long GetType();
    static Entity* create(Level* level);
};

#endif
