#ifndef GAME_CORE_MULE_HPP
#define GAME_CORE_MULE_HPP

#include "game/entity/mob/AbstractChestedHorse.hpp"

class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class SoundEvent;

class Mule : public AbstractChestedHorse
{
public:
    Mule(Level* level);

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual ResourceLocation* getDefaultLootTable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual void playChestEquipsSound();
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool HideUseTooltips();
    virtual unsigned long long GetType();
};

#endif
