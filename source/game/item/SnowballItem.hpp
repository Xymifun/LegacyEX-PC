#ifndef GAME_ITEM_SNOWBALLITEM_HPP
#define GAME_ITEM_SNOWBALLITEM_HPP

#include "game/item/Item.hpp"

class SnowballItem : public Item
{
public:
    SnowballItem();
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
