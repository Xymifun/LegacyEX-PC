#ifndef GAME_ENTITY_PROJECTILE_TIPPABLEARROW_HPP
#define GAME_ENTITY_PROJECTILE_TIPPABLEARROW_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/Arrow.hpp"

class CompoundTag;
class DataFixerUpper;
class ItemInstance;
class Level;
class LivingEntity;
class MobEffectInstance;
class Potion;

class TippableArrow : public Arrow
{
public:
    TippableArrow(Level* level);
    TippableArrow(Level* level, double x, double y, double z);
    TippableArrow(Level* level, boost::shared_ptr<LivingEntity> owner);

    static Entity* create(Level* level);
    static void staticCtor();
    void init();
    static int getCustomColor(const not_null_ptr<ItemInstance>& itemInstance);
    void updateColor();
    void setFixedColor(int color);
    void setEffectsFromItem(const not_null_ptr<ItemInstance>& itemInstance);
    void addEffect(MobEffectInstance* effect);
    virtual void defineSynchedData();
    int getColor();
    void makeParticle(int count);
    virtual void tick();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity);
    virtual not_null_ptr<ItemInstance> getPickupItem();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual unsigned long long GetType();

private:
    static const EntityDataAccessor<int>* DATA_EFFECT_COLOR_ID;

    Potion* mPotion;
    std::vector<MobEffectInstance*> mEffects;
    bool mFixedColor;
};

#endif
