#include "game/world/block/SnowBlock.hpp"

#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

SnowBlock::SnowBlock() : Block(Material::SNOW)
{
    DerivedInit();
    setTicking(true);
}

Item* SnowBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::snowball;
}

int SnowBlock::getResourceCount(Random* random)
{
    return 4;
}

void SnowBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random)
{
    if (level->getBrightness(LightLayer::BLOCK, pos) > 11)
    {
        spawnResources(level, pos, level->getBlockState(pos), 0);
        level->removeBlock(pos);
    }
}

bool SnowBlock::shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state)
{
    return level->getBrightness(LightLayer::BLOCK, pos) > 11;
}

bool SnowBlock::waterSpreadCausesSpawn()
{
    return false;
}
