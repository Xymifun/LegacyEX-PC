#ifndef GAME_CORE_FISH_HPP
#define GAME_CORE_FISH_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/entity/mob/WaterAnimal.hpp"

class DamageSource;
class DataFixerUpper;
class EntityTooltipDataHolder;
class Level;
class Player;
class SoundEvent;
class FishModel;

class Fish : public WaterAnimal
{
public:
    Fish(Level* level, bool deferredInit);
    virtual ~Fish() {}

    void _init();
    virtual void baseTick();
    virtual void registerGoals();
    virtual void registerAttributes();
    virtual float getEyeHeight();
    virtual bool breathesWater();
    virtual bool breathesAir();
    virtual const SoundEvent* getSwimSound();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual float getSoundVolume();
    virtual bool makeStepSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual int getExperienceReward(boost::shared_ptr<Player> player);
    virtual bool checkSpawnRules();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual unsigned long long GetType();

    static Entity* create(Level* level);
    static void addDataWalkers(DataFixerUpper* fixer);

protected:
    friend class FishModel;
    float mSwimRotation;
    float mSwimRotationOld;
    bool mSwimFlag;
    float mSwimSpeed;
    int mSwimTicks;
};

#endif
