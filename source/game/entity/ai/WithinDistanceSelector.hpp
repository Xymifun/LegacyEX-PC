#ifndef GAME_ENTITY_AI_WITHINDISTANCESELECTOR_HPP
#define GAME_ENTITY_AI_WITHINDISTANCESELECTOR_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/Predicate.hpp"

class Entity;

class WithinDistanceSelector : public Predicate<boost::shared_ptr<Entity> >
{
public:
    WithinDistanceSelector(double x, double y, double z, double distance)
        : x(x),
          y(y),
          z(z),
          distanceSqr(distance * distance)
    {
    }

    virtual bool apply(boost::shared_ptr<Entity> entity) const;
    virtual ~WithinDistanceSelector();

    double x;
    double y;
    double z;
    double distanceSqr;
};

#endif
