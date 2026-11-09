#include "game/world/block/OreBlock.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/Mth.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

OreBlock::OreBlock() : Block(Material::STONE)
{
    DerivedInit();
}

OreBlock::OreBlock(const MaterialColor* materialColor) : Block(Material::STONE, materialColor)
{
    DerivedInit();
}

Item* OreBlock::getResource(const BlockState*, Random*, int)
{
    if (this == Blocks::coal_ore)
    {
        return Items::coal;
    }
    if (this == Blocks::diamond_ore)
    {
        return Items::diamond;
    }
    if (this == Blocks::lapis_ore)
    {
        return Items::dye;
    }
    if (this == Blocks::emerald_ore)
    {
        return Items::emerald;
    }
    if (this == Blocks::quartz_ore)
    {
        return Items::quartz;
    }
    return Item::byBlock(this);
}

int OreBlock::getResourceCount(Random* random)
{
    if (this == Blocks::lapis_ore)
    {
        return random->nextInt(5) + 4;
    }
    return 1;
}

int OreBlock::getResourceCountForLootBonus(int lootBonus, Random* random)
{
    if (lootBonus > 0)
    {
        const arrayWithLength<BlockStateDefinition::BlockStateImpl*>* states =
            getBlockStateDefinition()->getPossibleBlockStates();
        if (Item::byBlock(this) != getResource(states->data[0], random, lootBonus))
        {
            int multiplier = random->nextInt(lootBonus + 2) - 1;
            if (multiplier < 0)
            {
                multiplier = 0;
            }
            return getResourceCount(random) * (multiplier + 1);
        }
    }
    return getResourceCount(random);
}

void OreBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance, int lootBonus)
{
    Block::spawnResources(level, pos, state, chance, lootBonus);

    if (getResource(state, level->mRandom, lootBonus) != Item::byBlock(this))
    {
        int experience = 0;
        if (this == Blocks::coal_ore)
        {
            experience = Mth::nextInt(level->mRandom, 0, 2);
        }
        else if (this == Blocks::diamond_ore || this == Blocks::emerald_ore)
        {
            experience = Mth::nextInt(level->mRandom, 3, 7);
        }
        else if (this == Blocks::lapis_ore || this == Blocks::quartz_ore)
        {
            experience = Mth::nextInt(level->mRandom, 2, 5);
        }
        popExperience(level, pos, experience);
    }
}

not_null_ptr<ItemInstance> OreBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(this));
}

int OreBlock::getSpawnResourcesAuxValue(const BlockState*)
{
    if (this != Blocks::lapis_ore)
    {
        return 0;
    }
    return DyeColor::BLUE->getItemData();
}
