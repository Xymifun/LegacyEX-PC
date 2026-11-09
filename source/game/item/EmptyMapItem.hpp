#ifndef GAME_ITEM_EMPTYMAPITEM_HPP
#define GAME_ITEM_EMPTYMAPITEM_HPP

#include "game/item/ComplexItem.hpp"

class ItemTooltipDataHolder;

class EmptyMapItem : public ComplexItem
{
public:
    EmptyMapItem();

    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
