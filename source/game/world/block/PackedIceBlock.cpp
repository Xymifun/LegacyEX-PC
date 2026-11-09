#include "game/world/block/PackedIceBlock.hpp"

#include "game/util/Random.hpp"
#include "game/world/block/Material.hpp"

PackedIceBlock::PackedIceBlock() : Block(Material::ICE_SOLID)
{
    DerivedInit();
    mFriction = 0.98f;
}

int PackedIceBlock::getResourceCount(Random* random)
{
    return 0;
}
