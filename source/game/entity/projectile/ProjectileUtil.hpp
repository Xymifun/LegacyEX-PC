#ifndef GAME_ENTITY_PROJECTILE_PROJECTILEUTIL_HPP
#define GAME_ENTITY_PROJECTILE_PROJECTILEUTIL_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

class Entity;
class HitResult;

class ProjectileUtil
{
public:
    static HitResult* forwardsRaycast(const boost::shared_ptr<Entity>& entity, bool checkEntities,
                                      const std::vector<boost::shared_ptr<Entity> >* ignoredEntities,
                                      const std::vector<unsigned long long>* ignoredEntityTypes);

    static void rotateTowardsMovement(const boost::shared_ptr<Entity>& entity, float rotationSpeed);
};

#endif
