#ifndef GAME_ITEM_ENCHANTMENTINSTANCE_HPP
#define GAME_ITEM_ENCHANTMENTINSTANCE_HPP

#include "game/item/WeighedRandomItem.hpp"

class Enchantment;

class EnchantmentInstance : public WeighedRandomItem
{
public:
    EnchantmentInstance(Enchantment* enchantment, int level);

    virtual ~EnchantmentInstance();

    EnchantmentInstance* copy();

    Enchantment* enchantment;
    int level;
};

#endif
