#ifndef GAME_CORE_MINECARTTNT_HPP
#define GAME_CORE_MINECARTTNT_HPP

#include "game/core/Minecart.hpp"

class BlockPos;
class BlockState;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Explosion;
class Level;

class MinecartTNT : public Minecart
{
public:
    MinecartTNT(Level* level);
    MinecartTNT(Level* level, double x, double y, double z);

    static Entity* create(Level* level);
    void _init();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual const Minecart::Type* getType();
    virtual const BlockState* getDefaultDisplayBlockState();
    virtual void tick();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void destroy(const DamageSource* source);
    virtual void explode(double power);
    virtual void causeFallDamage(float fallDistance, float damageMultiplier);
    virtual void activateMinecart(int x, int y, int z, bool powered);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void primeFuse();
    virtual int getFuse();
    virtual bool isPrimed();
    virtual float getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos);
    virtual bool shouldBlockExplode(Explosion* explosion, Level* level, const BlockPos& pos, float radius);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual unsigned long long GetType();

private:
    int mFuse;
};

#endif
