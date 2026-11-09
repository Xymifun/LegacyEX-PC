#ifndef GAME_CORE_EYEOFENDERSIGNAL_HPP
#define GAME_CORE_EYEOFENDERSIGNAL_HPP

#include <stdint.h>

#include "game/entity/mob/Entity.hpp"

class BlockPos;
class CompoundTag;
class Level;

class EyeOfEnderSignal : public Entity
{
public:
    EyeOfEnderSignal(Level* level, double x, double y, double z);
    EyeOfEnderSignal(Level* level);

    void _init();
    void signalTo(const BlockPos& pos);
    virtual void defineSynchedData();
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void lerpMotion(double xd, double yd, double zd);
    virtual void tick();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual float getBrightness();
    virtual int getLightColor();
    virtual bool isAttackable();
    static Entity* create(Level* level);
    virtual unsigned long long GetType();

private:
    int mUnknown;
    double mTargetX;
    double mTargetY;
    double mTargetZ;
    int mLife;
    bool mDropItem;
};

#endif
