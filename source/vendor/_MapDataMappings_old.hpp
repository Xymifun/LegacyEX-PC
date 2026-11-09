#ifndef SOURCE_VENDOR_MAPDATAMAPPINGS_OLD_HPP
#define SOURCE_VENDOR_MAPDATAMAPPINGS_OLD_HPP

#include "game/world/generation/PlayerUID.hpp"

class _MapDataMappings_old
{
public:
    PlayerUID xuids[256];
    unsigned char dimensions[32];

    _MapDataMappings_old();

    int getDimension(int id);
    void setMapping(int id, PlayerUID xuid, int dimension);
};

#endif
