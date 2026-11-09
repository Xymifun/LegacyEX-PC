#include "game/item/ShieldItem.hpp"

#include "game/item/BannerItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/Blocks.hpp"

void ShieldItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced)
{
    BannerItem::appendHoverTextFromBannerBlockEntityTag(itemInstance, lines);
}

int ShieldItem::getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const
{
    return UseAnim_block;
}

int ShieldItem::getUseDuration(not_null_ptr<ItemInstance> itemInstance)
{
    return 72000;
}

InteractionResultHolder ShieldItem::use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    player->startUsingItem(hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool ShieldItem::isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem)
{
    if (repairItem->getItem() == Item::byBlock(Blocks::planks))
    {
        return true;
    }

    return Item::isValidRepairItem(itemInstance, repairItem);
}
