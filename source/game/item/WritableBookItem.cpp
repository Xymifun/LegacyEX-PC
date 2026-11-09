#include "game/item/WritableBookItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/WrittenBookItem.hpp"
#include "game/world/generation/Player.hpp"

WritableBookItem::WritableBookItem() : Item()
{
    setMaxStackSize(1);
}

InteractionResultHolder WritableBookItem::use(Level* level, boost::shared_ptr<Player> player,
                                              InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    player->openItemInstanceGui(itemInstance, hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

bool WritableBookItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    return true;
}

bool WritableBookItem::makeSureTagIsValid(CompoundTag* tag)
{
    if (tag == NULL || !tag->contains(WrittenBookItem::TAG_PAGES, Tag::TAG_LIST))
    {
        return false;
    }

    ListTag* pages = tag->getList(WrittenBookItem::TAG_PAGES, Tag::TAG_STRING);
    for (int index = 0; index < pages->size(); ++index)
    {
        std::wstring page = pages->getString(index);
        if (page.size() > 256)
        {
            return false;
        }
    }
    return true;
}

unsigned int WritableBookItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return IDS_TOOLTIPS_OPEN;
}
