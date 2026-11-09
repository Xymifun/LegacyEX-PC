#include "game/item/AirItem.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Block.hpp"

AirItem::AirItem(Block* block) : mBlock(block)
{
}

int AirItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    return mBlock->getDescriptionId(-1);
}

void AirItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                              std::vector<HtmlString>* lines, bool advanced)
{
    Item::appendHoverText(itemInstance, player, lines, advanced);
    mBlock->appendHoverText(itemInstance, player, lines, advanced);
}
