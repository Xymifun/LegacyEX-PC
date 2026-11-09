#ifndef GAME_ITEM_ENCHANTMENTCATEGORY_HPP
#define GAME_ITEM_ENCHANTMENTCATEGORY_HPP

#include <vector>

class Item;

class EnchantmentCategory
{
public:
    static std::vector<const EnchantmentCategory*> CATEGORIES;

    static const EnchantmentCategory* ALL;
    static const EnchantmentCategory* ARMOR;
    static const EnchantmentCategory* ARMOR_FEET;
    static const EnchantmentCategory* ARMOR_LEGS;
    static const EnchantmentCategory* ARMOR_CHEST;
    static const EnchantmentCategory* ARMOR_HEAD;
    static const EnchantmentCategory* WEAPON;
    static const EnchantmentCategory* DIGGER;
    static const EnchantmentCategory* FISHING_ROD;
    static const EnchantmentCategory* BREAKABLE;
    static const EnchantmentCategory* BOW;
    static const EnchantmentCategory* WEARABLE;
    static const EnchantmentCategory* TRIDENT;

    EnchantmentCategory();

    virtual bool canEnchant(Item* item) const = 0;
};

#endif
