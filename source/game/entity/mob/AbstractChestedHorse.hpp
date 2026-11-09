#ifndef GAME_ENTITY_MOB_ABSTRACTCHESTEDHORSE_HPP
#define GAME_ENTITY_MOB_ABSTRACTCHESTEDHORSE_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EntityDataAccessor_Base;
class ItemInstance;
class Level;
class Player;
class SoundEvent;

template <class T> class EntityDataAccessor;

class AbstractChestedHorse : public AbstractHorse
{
public:
    AbstractChestedHorse(Level* level);
    virtual unsigned long long GetType();
    virtual ~AbstractChestedHorse() {}
    virtual void defineSynchedData();
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual double getRideHeight();
    virtual bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    virtual void registerAttributes();
    virtual void die(const DamageSource* source);
    virtual bool removeWhenFarAway();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual int getInventorySize();
    virtual const SoundEvent* getAngrySound();
    virtual void playChestEquipsSound();
    virtual int getInventoryColumns();

    bool hasChest();
    void setChest(bool chest);
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long type);
    static void staticCtor();

private:
    static const EntityDataAccessor<bool>* DATA_CHEST;
};

#endif
