#ifndef GAME_ENTITY_AI_ENTITYSELECTOR_HPP
#define GAME_ENTITY_AI_ENTITYSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;
class Player;

bool validPlayerPredicate(boost::shared_ptr<Player> player);
bool enemySelectorFunc(boost::shared_ptr<Entity> entity);
bool visibleEnemySelectorFunc(boost::shared_ptr<Entity> entity);

namespace EntitySelector
{
extern Predicate<boost::shared_ptr<Entity> >* NO_SPECTATORS;
extern Predicate<boost::shared_ptr<Entity> >* NO_CREATIVE_OR_SPECTATOR;
extern Predicate<boost::shared_ptr<Entity> >* CONTAINER_ENTITY_SELECTOR;
extern Predicate<boost::shared_ptr<Entity> >* NOT_BEING_RIDDEN_ENTITY_SELECTOR;
extern Predicate<boost::shared_ptr<Entity> >* ENTITY_STILL_ALIVE;
extern Predicate<boost::shared_ptr<Entity> >* ENEMY_SELECTOR;
extern Predicate<boost::shared_ptr<Entity> >* VISIBLE_ENEMY_SELECTOR;

Predicate<boost::shared_ptr<Entity> >* withinDistance(double x, double y, double z, double distance);
Predicate<boost::shared_ptr<Entity> >* riding(boost::shared_ptr<Entity> entity);
Predicate<boost::shared_ptr<Entity> >* notRiding(boost::shared_ptr<Entity> entity);
}

#endif
