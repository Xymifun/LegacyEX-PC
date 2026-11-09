#include "game/world/block/GasMaterial.hpp"

GasMaterial::~GasMaterial()
{
}

bool GasMaterial::isSolid()
{
    return false;
}

bool GasMaterial::blocksLight()
{
    return false;
}

bool GasMaterial::blocksMotion()
{
    return false;
}
