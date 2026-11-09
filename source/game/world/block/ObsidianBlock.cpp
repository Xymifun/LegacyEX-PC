#include "game/world/block/ObsidianBlock.hpp"

#include "game/item/Item.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

ObsidianBlock::ObsidianBlock() : Block(Material::STONE)
{
    DerivedInit();
}

Item* ObsidianBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Item::byBlock(Blocks::obsidian);
}

const MaterialColor* ObsidianBlock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::COLOR_BLACK;
}
