#ifndef GAME_CORE_ENDERCRYSTAL_HPP
#define GAME_CORE_ENDERCRYSTAL_HPP

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DamageSource;
class EntityTooltipDataHolder;
class Level;

template <class T> class EntityDataAccessor;

class EnderCrystal : public Entity
{
    friend class EnderCrystalRenderer;

public:
    EnderCrystal(Level* level);
    EnderCrystal(Level* level, double x, double y, double z);

    void _init(Level* level);
    static void staticCtor();
    virtual bool makeStepSound();
    virtual void defineSynchedData();
    virtual void tick();
    BlockPos* getBeamTarget();
    bool showsBottom();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setBeamTarget(BlockPos* pos);
    void setShowBottom(bool showBottom);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool isPickable();
    void onDestroyedBy(const DamageSource* source);
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void kill();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned long long GetType();
    static Entity* create(Level* level);

private:
    static const EntityDataAccessor<BlockPos*>* DATA_BEAM_TARGET;
    static const EntityDataAccessor<bool>* DATA_SHOW_BOTTOM;

    int mTime;
};

#endif
