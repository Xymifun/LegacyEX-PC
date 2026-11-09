#ifndef GAME_ITEM_EXPERIENCEBOTTLEITEM_HPP
#define GAME_ITEM_EXPERIENCEBOTTLEITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"

class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;

class ExperienceBottleItem : public Item
{
public:
    ExperienceBottleItem();
    virtual bool isFoil(not_null_ptr<ItemInstance> itemInstance);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
