#include "vendor/_MapDataMappings.hpp"

#include <cstring>

_MapDataMappings::_MapDataMappings()
{
    std::memset(xuids, 0, sizeof(xuids));
    std::memset(dimensions, 0, sizeof(dimensions));
}

int _MapDataMappings::getDimension(int id)
{
    int shift = (id % 4) * 2;
    unsigned int value = (dimensions[id >> 2] & (3 << shift)) >> shift;
    int dimension = 0;
    switch (value)
    {
    case 1:
        dimension = -1;
        break;
    case 2:
        dimension = 1;
        break;
    }
    return dimension;
}

void _MapDataMappings::setMapping(int id, PlayerUID xuid, int dimension)
{
    xuids[id] = xuid;

    int shift = (id % 4) * 2;
    dimensions[id >> 2] &= ~(3 << shift);
    switch (dimension)
    {
    case -1:
        dimensions[id >> 2] |= 1 << shift;
        break;
    case 1:
        dimensions[id >> 2] |= 2 << shift;
        break;
    default:
        break;
    }
}
