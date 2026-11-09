#ifndef GAME_ENTITY_MOB_MOBCACHE_HPP
#define GAME_ENTITY_MOB_MOBCACHE_HPP

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

class Mob;
class ServerLevel;

class MobCache : public boost::unordered_map<unsigned long long, boost::shared_ptr<Mob> >
{
public:
    MobCache();
    boost::shared_ptr<Mob> getTestMob(ServerLevel* level, unsigned long long entityType);
    void removeFromCache(boost::shared_ptr<Mob> mob);
};

#endif
