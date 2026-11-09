#include "game/item/PickaxeItem.hpp"

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

static arrayWithLength<Block*> BLOCKS(26, true);

void PickaxeItem::staticCtor()
{
    BLOCKS.data[0] = Blocks::activator_rail;
    BLOCKS.data[1] = Blocks::coal_ore;
    BLOCKS.data[2] = Blocks::cobblestone;
    BLOCKS.data[3] = Blocks::detector_rail;
    BLOCKS.data[4] = Blocks::diamond_block;
    BLOCKS.data[5] = Blocks::diamond_ore;
    BLOCKS.data[6] = Blocks::double_stone_slab;
    BLOCKS.data[7] = Blocks::golden_rail;
    BLOCKS.data[8] = Blocks::gold_block;
    BLOCKS.data[9] = Blocks::gold_ore;
    BLOCKS.data[10] = Blocks::ice;
    BLOCKS.data[11] = Blocks::iron_block;
    BLOCKS.data[12] = Blocks::iron_ore;
    BLOCKS.data[13] = Blocks::lapis_block;
    BLOCKS.data[14] = Blocks::lapis_ore;
    BLOCKS.data[15] = Blocks::lit_redstone_ore;
    BLOCKS.data[16] = Blocks::mossy_cobblestone;
    BLOCKS.data[17] = Blocks::netherrack;
    BLOCKS.data[18] = Blocks::packed_ice;
    BLOCKS.data[19] = Blocks::rail;
    BLOCKS.data[20] = Blocks::redstone_ore;
    BLOCKS.data[21] = Blocks::sandstone;
    BLOCKS.data[21] = Blocks::red_sandstone;
    BLOCKS.data[22] = Blocks::stone;
    BLOCKS.data[23] = Blocks::stone_slab;
    BLOCKS.data[24] = Blocks::stone_button;
    BLOCKS.data[25] = Blocks::stone_pressure_plate;
}

PickaxeItem::PickaxeItem(const Item::Tier* tier) : DiggerItem(2.0f, -2.8f, tier, BLOCKS)
{
}

bool PickaxeItem::canDestroySpecial(const BlockState* state)
{
    Block* block = state->getBlock();

    if (block == Blocks::obsidian)
    {
        return mTier->getLevel() == 3;
    }

    if (block == Blocks::diamond_block || block == Blocks::diamond_ore || block == Blocks::emerald_ore
        || block == Blocks::emerald_block || block == Blocks::gold_block || block == Blocks::gold_ore)
    {
        return mTier->getLevel() >= 2;
    }

    if (block == Blocks::iron_block || block == Blocks::iron_ore || block == Blocks::lapis_block
        || block == Blocks::lapis_ore)
    {
        return mTier->getLevel() >= 1;
    }

    if (block == Blocks::redstone_ore || block == Blocks::lit_redstone_ore)
    {
        return mTier->getLevel() >= 2;
    }

    Material* material = state->getMaterial();
    if (material == Material::STONE || material == Material::METAL || material == Material::HEAVY_METAL)
    {
        return true;
    }

    return false;
}

float PickaxeItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    Material* material = state->getMaterial();

    if (material == Material::METAL || material == Material::HEAVY_METAL || material == Material::STONE)
    {
        return mSpeed;
    }

    return DiggerItem::getDestroySpeed(itemInstance, state);
}
