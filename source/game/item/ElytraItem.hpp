#ifndef GAME_ITEM_ELYTRAITEM_HPP
#define GAME_ITEM_ELYTRAITEM_HPP

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

class ElytraItem : public Item
{
public:
    ElytraItem();

    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual TextureAtlasSprite* getIcon(not_null_ptr<ItemInstance> itemInstance);
    virtual int GetArmorType();
    virtual bool isValidRepairItem(not_null_ptr<ItemInstance> itemInstance, not_null_ptr<ItemInstance> repairItem);

    static bool isFlyEnabled(not_null_ptr<ItemInstance> itemInstance);

private:
    TextureAtlasSprite* mBrokenIcon;
};

#endif
