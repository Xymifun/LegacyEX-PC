#ifndef GAME_ITEM_BOTTLEITEM_HPP
#define GAME_ITEM_BOTTLEITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class IconRegister;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;
class TextureAtlasSprite;

class BottleItem : public Item
{
public:
    BottleItem();
    not_null_ptr<ItemInstance> turnBottleIntoItem(not_null_ptr<ItemInstance> itemInstance,
                                                  boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> result);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);
};

#endif
