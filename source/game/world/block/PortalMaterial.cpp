#include "game/world/block/PortalMaterial.hpp"

PortalMaterial::~PortalMaterial()
{
}

bool PortalMaterial::isSolid()
{
    return false;
}

bool PortalMaterial::blocksLight()
{
    return false;
}

bool PortalMaterial::blocksMotion()
{
    return false;
}
