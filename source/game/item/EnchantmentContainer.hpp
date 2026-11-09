#ifndef GAME_ITEM_ENCHANTMENTCONTAINER_HPP
#define GAME_ITEM_ENCHANTMENTCONTAINER_HPP

#include "game/item/SimpleContainer.hpp"

class EnchantmentMenu;

class EnchantmentContainer : public SimpleContainer
{
public:
    EnchantmentContainer(EnchantmentMenu* menu);
    virtual ~EnchantmentContainer();

    virtual int getMaxStackSize();
    virtual void setChanged();

private:
    EnchantmentMenu* mMenu;
};

#endif
