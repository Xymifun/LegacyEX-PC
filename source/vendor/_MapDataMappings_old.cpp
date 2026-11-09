#include "vendor/_MapDataMappings_old.hpp"

#include <cstring>

_MapDataMappings_old::_MapDataMappings_old()
{
    std::memset(xuids, 0, sizeof(xuids));
    std::memset(dimensions, 0, sizeof(dimensions));
}

int _MapDataMappings_old::getDimension(int id)
{
    return (dimensions[id >> 3] & (128 >> (id % 8))) ? -1 : 0;
}

void _MapDataMappings_old::setMapping(int id, PlayerUID xuid, int dimension)
{
    xuids[id] = xuid;

    if (dimension == 0)
        dimensions[id >> 3] &= ~(128 >> (id % 8));
    else
        dimensions[id >> 3] |= 128 >> (id % 8);
}
