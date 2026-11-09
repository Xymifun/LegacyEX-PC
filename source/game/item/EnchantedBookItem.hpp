#ifndef GAME_ITEM_ENCHANTEDBOOKITEM_HPP
#define GAME_ITEM_ENCHANTEDBOOKITEM_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class CompoundTag;
class Enchantment;
class EnchantmentInstance;
class ItemInstance;
class ListTag;
class Player;

class EnchantedBookItem : public Item
{
public:
    EnchantedBookItem();
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
    virtual bool isEnchantable(not_null_ptr<ItemInstance> itemInstance);
    static ListTag* getEnchantments(not_null_ptr<ItemInstance> itemInstance);
    virtual const Rarity* getRarity(not_null_ptr<ItemInstance> itemInstance);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    static void addEnchantment(not_null_ptr<ItemInstance> itemInstance, EnchantmentInstance* instance);
    static not_null_ptr<ItemInstance> createForEnchantment(EnchantmentInstance* instance);
    void createForEnchantment(Enchantment* enchantment, std::vector<not_null_ptr<ItemInstance> >* items);
};

#endif
