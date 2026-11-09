#ifndef GAME_ENTITY_PROJECTILE_SPECTRALARROW_HPP
#define GAME_ENTITY_PROJECTILE_SPECTRALARROW_HPP

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/Arrow.hpp"

class CompoundTag;
class DataFixerUpper;
class ItemInstance;
class Level;
class LivingEntity;

class SpectralArrow : public Arrow
{
public:
    SpectralArrow(Level* level);
    SpectralArrow(Level* level, boost::shared_ptr<LivingEntity> owner);
    SpectralArrow(Level* level, double x, double y, double z);

    static Entity* create(Level* level);
    void init();
    virtual void tick();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual not_null_ptr<ItemInstance> getPickupItem();
    virtual void doPostHurtEffects(const boost::shared_ptr<LivingEntity>& entity);
    virtual unsigned long long GetType();

private:
    int mDuration;
};

#endif
