#include "game/world/block/HardenedClayBlock.hpp"

#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/Material.hpp"

HardenedClayBlock::HardenedClayBlock() : Block(Material::STONE)
{
    DerivedInit();
}

const MaterialColor* HardenedClayBlock::getMapColor(const BlockState* blockState, LevelSource* levelSource,
                                                    const BlockPos& pos)
{
    return MaterialColor::COLOR_ORANGE;
}
