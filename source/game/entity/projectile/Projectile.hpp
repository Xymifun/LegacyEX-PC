#ifndef GAME_ENTITY_PROJECTILE_PROJECTILE_HPP
#define GAME_ENTITY_PROJECTILE_PROJECTILE_HPP

#include <boost/shared_ptr.hpp>

class Entity;
class HitResult;
class Level;
class LivingEntity;

class Projectile
{
public:
    Projectile(Level* level, const boost::shared_ptr<LivingEntity>& owner);
    virtual ~Projectile() {}

    virtual void shoot(double x, double y, double z, float velocity, float inaccuracy) = 0;
    virtual void shootFromRotation(boost::shared_ptr<Entity> entity, float xRot, float yRot, float offset,
                                   float velocity, float inaccuracy) = 0;
    virtual bool onHit(const HitResult* result);

protected:
    boost::shared_ptr<LivingEntity> mOwner;
    Level* mLevel;
};

#endif
