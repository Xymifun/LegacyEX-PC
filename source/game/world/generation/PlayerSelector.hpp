#ifndef GAME_WORLD_GENERATION_PLAYERSELECTOR_HPP
#define GAME_WORLD_GENERATION_PLAYERSELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;
class Player;

class PlayerSelector : public Predicate<boost::shared_ptr<Player> >
{
public:
    PlayerSelector(Entity* entity) : mEntity(entity) {}

    virtual bool apply(boost::shared_ptr<Player> player) const;
    virtual ~PlayerSelector() {}

    Entity* mEntity;
};

#endif
