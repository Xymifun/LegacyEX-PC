#include "game/item/AreaEffectCloud_Predicate.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/AreaEffectCloud.hpp"

bool AreaEffectCloud_Predicate::apply(boost::shared_ptr<Entity> entity) const
{
    boost::shared_ptr<AreaEffectCloud> cloud = boost::dynamic_pointer_cast<AreaEffectCloud>(entity);

    return cloud && cloud->isAlive() && cloud->getOwner()
           && eTYPE_DERIVED_FROM(EntityType::ENDER_DRAGON, cloud->getOwner()->GetType());
}
