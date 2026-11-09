#ifndef GAME_WORLD_SERVERLEVEL_HPP
#define GAME_WORLD_SERVERLEVEL_HPP

#include <boost/shared_ptr.hpp>

class Entity;

bool canSeeSkyFunction(const boost::shared_ptr<Entity>& entity);

#endif
