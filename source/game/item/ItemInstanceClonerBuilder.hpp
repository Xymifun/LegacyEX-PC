#ifndef GAME_ITEM_ITEMINSTANCECLONERBUILDER_HPP
#define GAME_ITEM_ITEMINSTANCECLONERBUILDER_HPP

#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"

class ItemInstanceClonerBuilder : public UIScene_CreativeMenu::ItemBuilder
{
public:
    ItemInstanceClonerBuilder(const ItemInstance& item) : mItem(item) {}

    virtual not_null_ptr<ItemInstance> build() const;

protected:
    ItemInstance mItem;
};

#endif
