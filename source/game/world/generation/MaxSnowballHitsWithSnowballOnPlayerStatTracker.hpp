#ifndef GAME_WORLD_GENERATION_MAXSNOWBALLHITSWITHSNOWBALLONPLAYERSTATTRACKER_HPP
#define GAME_WORLD_GENERATION_MAXSNOWBALLHITSWITHSNOWBALLONPLAYERSTATTRACKER_HPP

#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerUID.hpp"

class DamageEvent;

class MaxSnowballHitsWithSnowballOnPlayerStatTracker : public StatTracker<unsigned int>
{
public:
    MaxSnowballHitsWithSnowballOnPlayerStatTracker(const StatsUID& statsUID);
    virtual void update(const DamageEvent* damageEvent);

private:
    StatsUID mPlayerUID;
    boost::unordered_map<StatsUID, unsigned int, boost::hash<StatsUID>, std::equal_to<StatsUID> > mSnowballHits;
};

#endif
