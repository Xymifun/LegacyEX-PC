#include "game/world/block/LiquidMaterial.hpp"

LiquidMaterial::~LiquidMaterial()
{
}

bool LiquidMaterial::isLiquid()
{
    return true;
}

bool LiquidMaterial::isSolid()
{
    return false;
}

bool LiquidMaterial::blocksMotion()
{
    return false;
}
