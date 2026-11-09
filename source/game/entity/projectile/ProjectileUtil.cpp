#include "game/entity/projectile/ProjectileUtil.hpp"

#include <cmath>

#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/HitResult.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"


HitResult* ProjectileUtil::forwardsRaycast(const boost::shared_ptr<Entity>& entity, bool checkEntities,
                                           const std::vector<boost::shared_ptr<Entity> >* ignoredEntities,
                                           const std::vector<unsigned long long>* ignoredEntityTypes)
{
    double x = entity->mX;
    double y = entity->mY;
    double z = entity->mZ;
    double xd = entity->mXd;
    double yd = entity->mYd;
    double zd = entity->mZd;
    Level* level = entity->mLevel;

    Vec3* from = Vec3::newTemp(x, y, z);
    Vec3* to = Vec3::newTemp(x + xd, y + yd, z + zd);

    HitResult* hitResult = level->clip(from, to, false, true, false, false, NULL, false);

    if (checkEntities)
    {
        if (hitResult != NULL)
        {
            to = Vec3::newTemp(hitResult->mLocation->x, hitResult->mLocation->y, hitResult->mLocation->z);
        }

        boost::shared_ptr<Entity> hitEntity;
        double nearestDistance = 0.0;

        AABB* searchBox = entity->mBoundingBox->expand(xd, yd, zd)->grow(1.0);
        std::vector<boost::shared_ptr<Entity> >* candidates = level->getEntities(entity, searchBox, NULL);

        for (std::vector<boost::shared_ptr<Entity> >::const_iterator it = candidates->begin(); it != candidates->end();
             ++it)
        {
            boost::shared_ptr<Entity> candidate = *it;

            if (!candidate->isPickable() || candidate->mNoPhysics)
            {
                continue;
            }

            if (ignoredEntities != NULL)
            {
                bool ignored = false;

                for (std::vector<boost::shared_ptr<Entity> >::const_iterator ignoredIt = ignoredEntities->begin();
                     ignoredIt != ignoredEntities->end(); ++ignoredIt)
                {
                    if (candidate->is(*ignoredIt))
                    {
                        ignored = true;
                        break;
                    }
                }

                if (ignored)
                {
                    continue;
                }
            }

            if (ignoredEntityTypes != NULL)
            {
                bool ignored = false;

                for (std::vector<unsigned long long>::const_iterator typeIt = ignoredEntityTypes->begin();
                     typeIt != ignoredEntityTypes->end(); ++typeIt)
                {
                    unsigned long long ignoredType = *typeIt;
                    unsigned long long candidateType = candidate->GetType();

                    if ((ignoredType & 0x3F) == 0)
                    {
                        if ((candidateType & ignoredType) == ignoredType)
                        {
                            ignored = true;
                            break;
                        }
                    }
                    else if (candidateType == ignoredType)
                    {
                        ignored = true;
                        break;
                    }
                }

                if (ignored)
                {
                    continue;
                }
            }

            HitResult* candidateHit = candidate->mBoundingBox->grow(0.30000001192092896)->clip(from, to);

            if (candidateHit != NULL)
            {
                double distance = from->distanceToSqr(candidateHit->mLocation);

                if (distance < nearestDistance || nearestDistance == 0.0)
                {
                    hitEntity = candidate;
                    nearestDistance = distance;
                }
            }
        }

        if (hitEntity)
        {
            hitResult = new HitResult(hitEntity);
        }
    }

    return hitResult;
}

void ProjectileUtil::rotateTowardsMovement(const boost::shared_ptr<Entity>& entity, float rotationSpeed)
{
    double xd = entity->mXd;
    double yd = entity->mYd;
    double zd = entity->mZd;

    double horizontalDistance = ::sqrt(xd * xd + zd * zd);

    entity->mYRot = (float)(::atan2(zd, xd) * Mth::RAD_TO_DEG) + 90.0f;
    entity->mXRot = (float)(::atan2(horizontalDistance, yd) * Mth::RAD_TO_DEG) - 90.0f;

    while (entity->mXRot - entity->mXRotO < -180.0f)
    {
        entity->mXRotO -= 360.0f;
    }

    while (entity->mXRot - entity->mXRotO >= 180.0f)
    {
        entity->mXRotO += 360.0f;
    }

    while (entity->mYRot - entity->mYRotO < -180.0f)
    {
        entity->mYRotO -= 360.0f;
    }

    while (entity->mYRot - entity->mYRotO >= 180.0f)
    {
        entity->mYRotO += 360.0f;
    }

    entity->mXRot = entity->mXRotO + (entity->mXRot - entity->mXRotO) * rotationSpeed;
    entity->mYRot = entity->mYRotO + (entity->mYRot - entity->mYRotO) * rotationSpeed;
}
