#ifndef GAME_ITEM_COALITEM_HPP
#define GAME_ITEM_COALITEM_HPP

#include "game/item/Item.hpp"

class IconRegister;
class ItemInstance;
class TextureAtlasSprite;

class CoalItem : public Item
{
public:
    CoalItem();
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mCharcoalIcon;
};

#endif
