#include "game/world/block/BlueIceBlock.hpp"

#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

BlueIceBlock::BlueIceBlock(Material* material) : Block(material)
{
    setIconName(L"blue_ice");
    DerivedInit();
    mFriction = 0.989f;
}

int BlueIceBlock::getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue)
{
    return 0x00F000F0;
}

int BlueIceBlock::getResourceCount(Random* random)
{
    return 0;
}
