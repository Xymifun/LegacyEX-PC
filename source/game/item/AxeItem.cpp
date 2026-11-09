#include "game/item/AxeItem.hpp"

#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

static arrayWithLength<Block*> BLOCKS(22, false);

void AxeItem::staticCtor()
{
    BLOCKS.data[0] = Blocks::planks;
    BLOCKS.data[1] = Blocks::bookshelf;
    BLOCKS.data[2] = Blocks::log;
    BLOCKS.data[3] = Blocks::log2;
    BLOCKS.data[4] = Blocks::chest;
    BLOCKS.data[5] = Blocks::pumpkin;
    BLOCKS.data[6] = Blocks::carved_pumpkin;
    BLOCKS.data[7] = Blocks::lit_pumpkin;
    BLOCKS.data[8] = Blocks::melon_block;
    BLOCKS.data[9] = Blocks::ladder;
    BLOCKS.data[10] = Blocks::wooden_button;
    BLOCKS.data[11] = Blocks::acacia_button;
    BLOCKS.data[12] = Blocks::birch_button;
    BLOCKS.data[13] = Blocks::dark_oak_button;
    BLOCKS.data[14] = Blocks::jungle_button;
    BLOCKS.data[15] = Blocks::spruce_button;
    BLOCKS.data[16] = Blocks::wooden_pressure_plate;
    BLOCKS.data[17] = Blocks::acacia_pressure_plate;
    BLOCKS.data[18] = Blocks::birch_pressure_plate;
    BLOCKS.data[19] = Blocks::dark_oak_pressure_plate;
    BLOCKS.data[20] = Blocks::jungle_pressure_plate;
    BLOCKS.data[21] = Blocks::spruce_pressure_plate;
}

AxeItem::AxeItem(const Item::Tier* tier) : DiggerItem(tier, BLOCKS)
{
    mAttackDamage = 3.0f + tier->getAttackDamageBonus();
}

float AxeItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    Material* material = state->getMaterial();
    if (material == Material::WOOD || material == Material::PLANT || material == Material::REPLACEABLE_PLANT
        || material == Material::BAMBOO)
    {
        return mSpeed;
    }
    return DiggerItem::getDestroySpeed(itemInstance, state);
}

InteractionResult::EInteractionResult AxeItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                     const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                     const Direction* direction, float hitX, float hitY, float hitZ,
                                                     bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (block->isStrippable() && !Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!simulate)
        {
            const BlockState* strippedState = block->getStrippedBlock(state);
            level->setBlock(pos, strippedState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            if (!level->mIsClientSide)
            {
                itemInstance->hurtAndBreak(1, player);
            }
        }
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Pass;
}
