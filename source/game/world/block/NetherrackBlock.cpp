#include "game/world/block/NetherrackBlock.hpp"

#include "game/world/LevelSource.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MaterialColor.hpp"

NetherrackBlock::NetherrackBlock() : Block(Material::STONE)
{
    DerivedInit();
}

const MaterialColor* NetherrackBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                                  const BlockPos& pos)
{
    return MaterialColor::NETHER;
}
