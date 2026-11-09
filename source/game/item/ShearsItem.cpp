#include "game/item/ShearsItem.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

ShearsItem::ShearsItem() : Item()
{
    setMaxStackSize(1);
    setMaxDamage(238);
}

bool ShearsItem::mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state,
                           const BlockPos& pos, boost::shared_ptr<LivingEntity> entity)
{
    if (!level->mIsClientSide)
    {
        itemInstance->hurtAndBreak(1, entity);
    }

    Block* block = state->getBlock();

    if (state->getMaterial() == Material::LEAVES || block == Blocks::web || block == Blocks::tallgrass
        || block == Blocks::vine || block == Blocks::tripwire || block == Blocks::wool || block == Blocks::sea_grass)
    {
        return true;
    }

    return Item::mineBlock(itemInstance, level, state, pos, entity);
}

bool ShearsItem::canDestroySpecial(const BlockState* state)
{
    Block* block = state->getBlock();
    return block == Blocks::web || block == Blocks::redstone_wire || block == Blocks::tripwire;
}

float ShearsItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    Block* block = state->getBlock();

    if (block == Blocks::web || block->mMaterial == Material::LEAVES)
    {
        return 15.0f;
    }

    if (block == Blocks::wool)
    {
        return 5.0f;
    }

    return Item::getDestroySpeed(itemInstance, state);
}

InteractionResult::EInteractionResult ShearsItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    Block* block = level->getBlockState(pos)->getBlock();

    if (block == Blocks::pumpkin)
    {
        if (direction == Direction::UP || direction == Direction::DOWN)
        {
            return InteractionResult::eInteractionResult_Fail;
        }

        if (!simulate)
        {
            Block* carvedPumpkin = Blocks::carved_pumpkin;
            int data = getLevelDataForAuxValue(itemInstance->getAuxValue());
            const BlockState* carvedState = carvedPumpkin->getStateForPlacement(level, pos, direction, hitX, hitY, hitZ,
                                                                                data, player);
            level->playSound(player, pos, SoundEvents::PUMPKIN_CARVE, SoundSource::eSoundSource_Players, 1.0f, 1.0f,
                             16.0f);
            level->setBlock(pos, carvedState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            Block::popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Items::pumpkinSeeds, 1, 0)));
            itemInstance->hurtAndBreak(1, player);
        }

        return InteractionResult::eInteractionResult_Success;
    }

    return InteractionResult::eInteractionResult_Pass;
}
