#ifndef GAME_ITEM_CLOCKITEM_HPP
#define GAME_ITEM_CLOCKITEM_HPP

#include "game/item/Item.hpp"

class IconRegister;
class TextureAtlasSprite;

class ClockItem : public Item
{
public:
    ClockItem();
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mIcons[4];
};

#endif
