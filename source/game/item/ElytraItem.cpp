#include "game/item/ElytraItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/BehaviorRegistry.hpp"
#include "game/core/IconRegister.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/ElytraItem_ItemPropertyFunction.hpp"
#include "game/world/block/DispenserBlock.hpp"

ElytraItem::ElytraItem() : Item()
{
    mMaxStackSize = 1;
    setMaxDamage(0x1B0);
    addProperty(ResourceLocation(L"broken"), new ElytraItem_ItemPropertyFunction());
    DispenserBlock::DISPENSER_REGISTRY.add(this, ArmorItem::DISPENSE_ITEM_BEHAVIOR);
}

int ElytraItem::GetArmorType()
{
    return 2;
}

bool ElytraItem::isFlyEnabled(not_null_ptr<ItemInstance> itemInstance)
{
    return itemInstance->getDamageValue() < itemInstance->getMaxDamage() - 1;
}

bool ElytraItem::isValidRepairItem(not_null_ptr<ItemInstance>, not_null_ptr<ItemInstance> repairItem)
{
    return repairItem->getItem() == Items::phantomMembrane;
}

InteractionResultHolder ElytraItem::use(Level*, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInHand = player->getItemInHand(hand);
    const EquipmentSlot* slot = Mob::getEquipmentSlotForItem(itemInHand);
    not_null_ptr<ItemInstance> equippedItem = player->getItemSlot(slot);

    player->setItemSlot(slot, itemInHand->copy());
    itemInHand->setCount(0);

    if (!equippedItem->isEmpty())
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Success, equippedItem);
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInHand);
}

bool ElytraItem::TestUse(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return true;
}

unsigned int ElytraItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    if (tooltipData.m_eInteractionResult != InteractionResult::eInteractionResult_Fail)
    {
        return IDS_TOOLTIPS_EQUIP;
    }

    return IDS_NONE;
}

void ElytraItem::registerIcons(IconRegister* iconRegister)
{
    Item::registerIcons(iconRegister);
    mBrokenIcon = iconRegister->registerIcon(L"broken_elytra");
}

TextureAtlasSprite* ElytraItem::getLayerIcon(int auxValue, int, not_null_ptr<ItemInstance>)
{
    if (auxValue < getMaxDamage() - 1)
    {
        return mIcon;
    }

    return mBrokenIcon;
}

TextureAtlasSprite* ElytraItem::getIcon(not_null_ptr<ItemInstance> itemInstance)
{
    bool flyEnabled = isFlyEnabled(itemInstance);
    return flyEnabled ? mIcon : mBrokenIcon;
}
