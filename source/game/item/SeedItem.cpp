#include "game/item/SeedItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

SeedItem::SeedItem(Block* cropBlock, Block* soilBlock) : Item(), mCropBlock(cropBlock), mSoilBlock(soilBlock)
{
}

InteractionResult::EInteractionResult SeedItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                      const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                      const Direction* direction, float hitX, float hitY, float hitZ,
                                                      bool simulate)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (direction != Direction::UP || !player->mayUseItemAt(pos.relative(direction), direction, itemInstance)
        || level->getBlockState(pos)->getBlock() != mSoilBlock || !level->isEmptyBlock(pos.above()))
    {
        return InteractionResult::eInteractionResult_Fail;
    }
    if (!simulate)
    {
        level->setBlockAndUpdate(pos.above(), mCropBlock->defaultBlockState());
        itemInstance->shrink(1);
    }
    return InteractionResult::eInteractionResult_Success;
}
