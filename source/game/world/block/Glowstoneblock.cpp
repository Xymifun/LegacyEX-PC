#include "game/world/block/Glowstoneblock.hpp"

#include "game/core/Mth.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

Glowstoneblock::Glowstoneblock(Material* material) : Block(material)
{
    DerivedInit();
}

int Glowstoneblock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return Mth::clamp(getResourceCount(random) + random->nextInt(lootBonus + 1), 1, 4);
}

int Glowstoneblock::getResourceCount(Random* random)
{
    return random->nextInt(3) + 2;
}

Item* Glowstoneblock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::glowstoneDust;
}

const MaterialColor* Glowstoneblock::getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return MaterialColor::SAND;
}
