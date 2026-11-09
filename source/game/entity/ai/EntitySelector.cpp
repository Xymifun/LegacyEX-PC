#include "game/entity/ai/EntitySelector.hpp"

#include "game/entity/ai/RidingSelector.hpp"
#include "game/entity/ai/WithinDistanceSelector.hpp"
#include "game/entity/ai/NoSpectatorsSelector.hpp"
#include "game/entity/ai/NoCreativeOrSpectatorSelector.hpp"
#include "game/entity/ai/AliveEntitySelector.hpp"
#include "game/entity/ai/NotBeingRiddenEntitySelector.hpp"
#include "game/entity/ai/ContainerEntitySelector.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/generation/Player.hpp"

bool validPlayerPredicate(boost::shared_ptr<Player> player)
{
    static Predicate<boost::shared_ptr<Entity> >* const validPlayerDistance = EntitySelector::withinDistance(
        0.0, 128.0, 0.0, 192.0);

    return EntitySelector::ENTITY_STILL_ALIVE->apply(player) && validPlayerDistance->apply(player);
}

bool enemySelectorFunc(boost::shared_ptr<Entity> entity)
{
    return eTYPE_DERIVED_FROM(EntityType::ENEMY, entity->GetType());
}

bool visibleEnemySelectorFunc(boost::shared_ptr<Entity> entity)
{
    return eTYPE_DERIVED_FROM(EntityType::ENEMY, entity->GetType()) && !entity->isInvisible();
}

namespace EntitySelector
{
Predicate<boost::shared_ptr<Entity> >* ENTITY_STILL_ALIVE = new AliveEntitySelector();
Predicate<boost::shared_ptr<Entity> >* NOT_BEING_RIDDEN_ENTITY_SELECTOR = new NotBeingRiddenEntitySelector();
Predicate<boost::shared_ptr<Entity> >* CONTAINER_ENTITY_SELECTOR = new ContainerEntitySelector();
Predicate<boost::shared_ptr<Entity> >* NO_CREATIVE_OR_SPECTATOR = new NoCreativeOrSpectatorSelector();
Predicate<boost::shared_ptr<Entity> >* NO_SPECTATORS = new NoSpectatorsSelector();
Predicate<boost::shared_ptr<Entity> >* ENEMY_SELECTOR = new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(
    enemySelectorFunc);
Predicate<boost::shared_ptr<Entity> >* VISIBLE_ENEMY_SELECTOR =
    new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(visibleEnemySelectorFunc);
}

namespace EntitySelector
{

Predicate<boost::shared_ptr<Entity> >* withinDistance(double x, double y, double z, double distance)
{
    return new WithinDistanceSelector(x, y, z, distance);
}

Predicate<boost::shared_ptr<Entity> >* riding(boost::shared_ptr<Entity> entity)
{
    return new RidingSelector(entity, true);
}

Predicate<boost::shared_ptr<Entity> >* notRiding(boost::shared_ptr<Entity> entity)
{
    return new RidingSelector(entity, false);
}

}
