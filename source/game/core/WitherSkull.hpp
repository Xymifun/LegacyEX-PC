#ifndef GAME_CORE_WITHERSKULL_HPP
#define GAME_CORE_WITHERSKULL_HPP

#include <vector>
#include "game/entity/projectile/Fireball.hpp"

class BlockPos;
class Explosion;
class HitResult;

class WitherSkull : public Fireball
{
public:
    explicit WitherSkull(Level* level);
    WitherSkull(Level* level, boost::shared_ptr<LivingEntity> owner, double x, double y, double z);
    WitherSkull(Level* level, double x, double y, double z, double xd, double yd, double zd);
    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);
    bool isOnFire();
    float getBlockExplosionResistance(Explosion* explosion, Level* level, const BlockPos& pos);
    bool isPickable();
    virtual bool isDangerous();
    virtual void setDangerous(bool dangerous);
    float getInertia();
    void onHit(const HitResult* result);
    void defineSynchedData();
    bool shouldBurn();
    std::vector<unsigned long long>* getEntityTypesToIgnoreCollisionsWith();
    unsigned long long GetType();

private:
    static const EntityDataAccessor<bool>* DATA_DANGEROUS;
};

#endif
