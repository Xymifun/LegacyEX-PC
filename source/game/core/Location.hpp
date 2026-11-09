#ifndef GAME_CORE_LOCATION_HPP
#define GAME_CORE_LOCATION_HPP

#include "game/core/Position.hpp"

class Level;

class Location : public Position
{
public:
    virtual ~Location() {}

    virtual Level* getWorld() = 0;
};

#endif
