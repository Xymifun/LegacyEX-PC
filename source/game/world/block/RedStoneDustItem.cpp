#include "game/world/block/RedStoneDustItem.hpp"

#include "game/core/GenericStats.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

RedStoneDustItem::RedStoneDustItem() : Item()
{
}

InteractionResult::EInteractionResult RedStoneDustItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                              const BlockPos& pos,
                                                              InteractionHand::EInteractionHand hand,
                                                              const Direction* direction, float hitX, float hitY,
                                                              float hitZ, bool simulate)
{
    BlockPos placePos = level->getBlockState(pos)->getBlock()->mayReplaceWithPlace(level, pos)
                            ? pos
                            : pos.relative(direction);

    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(placePos, direction, itemInstance)
        || !level->mayPlace(level->getBlockState(placePos)->getBlock(), placePos, false, direction,
                            boost::shared_ptr<Entity>())
        || !Blocks::redstone_wire->mayPlace(level, placePos))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    if (!simulate)
    {
        player->awardStat(GenericStats::blocksPlaced(Blocks::redstone_wire),
                          GenericStats::param_blocksPlaced(Blocks::redstone_wire->defaultBlockState(), 1));
        itemInstance->shrink(1);
        level->setBlockAndUpdate(placePos, Blocks::redstone_wire->defaultBlockState());
    }

    return InteractionResult::eInteractionResult_Success;
}
