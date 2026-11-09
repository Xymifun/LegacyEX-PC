#include "game/world/block/SeaLanternBlock.hpp"

#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"
#include "game/item/Items.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MaterialColor.hpp"

SeaLanternBlock::SeaLanternBlock(Material* material) : Block(material)
{
    DerivedInit();
}

int SeaLanternBlock::getResourceCount(Random* random)
{
    return random->nextInt(2) + 2;
}

int SeaLanternBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    return Mth::clamp(getResourceCount(random) + random->nextInt(lootBonus + 1), 1, 5);
}

Item* SeaLanternBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::prismarineCrystals;
}

const MaterialColor* SeaLanternBlock::getMapColor(const BlockState* state, LevelSource* levelSource,
                                                  const BlockPos& pos)
{
    return MaterialColor::QUARTZ;
}

bool SeaLanternBlock::isSilkTouchable()
{
    return true;
}
