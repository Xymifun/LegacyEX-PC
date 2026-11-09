#ifndef VENDOR__MAPDATAMAPPINGS_HPP
#define VENDOR__MAPDATAMAPPINGS_HPP

#include "game/world/generation/PlayerUID.hpp"

class _MapDataMappings
{
public:
    PlayerUID xuids[256];
    unsigned char dimensions[64];

    _MapDataMappings();

    int getDimension(int id);
    void setMapping(int id, PlayerUID xuid, int dimension);
};

#endif
