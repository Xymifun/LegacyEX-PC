#include "game/world/block/PoweredBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

PoweredBlock::PoweredBlock(Material* material, const MaterialColor* materialColor) : Block(material, materialColor)
{
    DerivedInit();
}

int PoweredBlock::getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                            const Direction* direction)
{
    return Block::SIGNAL_MAX;
}

bool PoweredBlock::isSignalSource(const BlockState* state)
{
    return true;
}
