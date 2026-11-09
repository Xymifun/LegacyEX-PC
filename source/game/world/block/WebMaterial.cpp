#include "game/world/block/WebMaterial.hpp"

WebMaterial::~WebMaterial()
{
}

bool WebMaterial::isSolid()
{
    return false;
}

bool WebMaterial::blocksMotion()
{
    return false;
}
