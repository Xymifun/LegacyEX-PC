#ifndef GAME_CORE_SHULKERBULLET_HPP
#define GAME_CORE_SHULKERBULLET_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DamageSource;
class HitResult;
class Level;
class LivingEntity;

class ShulkerBullet : public Entity
{
public:
    ShulkerBullet(Level* level);
    ShulkerBullet(Level* level, double x, double y, double z, double xd, double yd, double zd);
    ShulkerBullet(Level* level, boost::shared_ptr<LivingEntity> owner, boost::shared_ptr<Entity> target,
                  const Direction::Axis* axis);

    static Entity* create(Level* level);
    void init();
    void setMoveDirection(const Direction* direction);
    const Direction* getMoveDirection();
    void selectNextMoveDirection(const Direction::Axis* axis);
    void onHit(const HitResult* result);
    virtual void tick();
    virtual SoundSource::ESoundSource getSoundSource();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual float getBrightness();
    virtual int getLightColor();
    virtual bool isPickable();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual bool isOnFire();
    virtual void defineSynchedData();
    virtual unsigned long long GetType();

private:
    boost::shared_ptr<LivingEntity> mOwner;
    boost::shared_ptr<Entity> mTarget;
    const Direction* mMoveDirection;
    int mSteps;
    double mPowerX;
    double mPowerY;
    double mPowerZ;
    std::wstring mOwnerUniqueId;
    BlockPos mOwnerPos;
    std::wstring mTargetUniqueId;
    BlockPos mTargetPos;
};

#endif
