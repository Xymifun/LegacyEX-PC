#include "game/item/EnchantedBookItem.hpp"

#include "game/core/Rarity.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentInstance.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

static const std::wstring TAG_STORED_ENCHANTMENTS = L"StoredEnchantments";
const wchar_t* TAG_ENCH_ID = L"id";
const wchar_t* TAG_ENCH_LEVEL = L"lvl";

EnchantedBookItem::EnchantedBookItem() : Item()
{
}

bool EnchantedBookItem::isFoil(not_null_ptr<ItemInstance> itemInstance)
{
    return true;
}

bool EnchantedBookItem::isEnchantable(not_null_ptr<ItemInstance> itemInstance)
{
    return false;
}

ListTag* EnchantedBookItem::getEnchantments(not_null_ptr<ItemInstance> itemInstance)
{
    CompoundTag* tag = itemInstance->getTag();
    if (tag != NULL)
    {
        return tag->getList(TAG_STORED_ENCHANTMENTS, Tag::TAG_COMPOUND);
    }
    return new ListTag();
}

const Rarity* EnchantedBookItem::getRarity(not_null_ptr<ItemInstance> itemInstance)
{
    ListTag* enchantments = getEnchantments(itemInstance);
    if (enchantments != NULL && enchantments->isEmpty())
    {
        return Item::getRarity(itemInstance);
    }
    return Rarity::UNCOMMON;
}

void EnchantedBookItem::appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                        std::vector<HtmlString>* lines, bool advanced)
{
    Item::appendHoverText(itemInstance, player, lines, advanced);
    ListTag* enchantments = getEnchantments(itemInstance);
    if (enchantments != NULL)
    {
        std::wstring unformatted = L"";
        for (int i = 0; i < enchantments->size(); ++i)
        {
            int id = enchantments->getCompound(i)->getShort(TAG_ENCH_ID);
            int level = enchantments->getCompound(i)->getShort(TAG_ENCH_LEVEL);
            if (Enchantment::byId(id) != NULL)
            {
                lines->push_back(Enchantment::byId(id)->getFullname(level));
            }
        }
    }
}

void EnchantedBookItem::addEnchantment(not_null_ptr<ItemInstance> itemInstance, EnchantmentInstance* instance)
{
    ListTag* enchantments = getEnchantments(itemInstance);
    bool add = true;
    for (int i = 0; i < enchantments->size(); ++i)
    {
        CompoundTag* entry = enchantments->getCompound(i);
        if (Enchantment::byId(entry->getShort(TAG_ENCH_ID)) == instance->enchantment)
        {
            if (entry->getShort(TAG_ENCH_LEVEL) < instance->level)
            {
                entry->putShort(TAG_ENCH_LEVEL, static_cast<short>(instance->level));
            }
            add = false;
            break;
        }
    }
    if (add)
    {
        CompoundTag* entry = new CompoundTag();
        entry->putShort(TAG_ENCH_ID, static_cast<short>(Enchantment::getId(instance->enchantment)));
        entry->putShort(TAG_ENCH_LEVEL, static_cast<short>(instance->level));
        enchantments->add(entry);
    }
    if (!itemInstance->hasTag())
    {
        itemInstance->setTag(new CompoundTag());
    }
    itemInstance->getTag()->put(TAG_STORED_ENCHANTMENTS.c_str(), enchantments);
}

not_null_ptr<ItemInstance> EnchantedBookItem::createForEnchantment(EnchantmentInstance* instance)
{
    not_null_ptr<ItemInstance> item(new ItemInstance(Items::enchantedBook));
    addEnchantment(item, instance);
    return item;
}

void EnchantedBookItem::createForEnchantment(Enchantment* enchantment, std::vector<not_null_ptr<ItemInstance> >* items)
{
    for (int level = enchantment->getMinLevel(); level <= enchantment->getMaxLevel(); ++level)
    {
        items->push_back(createForEnchantment(new EnchantmentInstance(enchantment, level)));
    }
}
