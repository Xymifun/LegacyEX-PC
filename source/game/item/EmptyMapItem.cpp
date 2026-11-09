#include "game/item/EmptyMapItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/GenericStats.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MapItem.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"

EmptyMapItem::EmptyMapItem() : ComplexItem()
{
}

InteractionResultHolder EmptyMapItem::use(Level* level, boost::shared_ptr<Player> player,
                                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    not_null_ptr<ItemInstance> map = MapItem::setupNewMap(player->mXuid, level, player->mDimension, player->mX,
                                                          player->mZ, 3, true, false);
    itemInstance->shrink(1);

    if (itemInstance->isEmpty())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, map);
    }

    if (!player->mInventory->add(map->copy()))
    {
        player->drop(map, false);
    }

    player->awardStat(GenericStats::itemsUsed(this), GenericStats::param_itemsUsed(player, itemInstance));

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int EmptyMapItem::GetUseTooltip(const ItemTooltipDataHolder&)
{
    return IDS_TOOLTIPS_USE;
}
