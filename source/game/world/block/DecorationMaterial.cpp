#include "game/world/block/DecorationMaterial.hpp"

DecorationMaterial::~DecorationMaterial()
{
}

bool DecorationMaterial::isSolid()
{
    return false;
}

bool DecorationMaterial::blocksLight()
{
    return false;
}

bool DecorationMaterial::blocksMotion()
{
    return false;
}
