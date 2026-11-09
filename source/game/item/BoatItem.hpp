#ifndef GAME_ITEM_BOATITEM_HPP
#define GAME_ITEM_BOATITEM_HPP

#include "game/item/Item.hpp"
#include "game/core/Boat.hpp"

class BoatItem : public Item
{
public:
    explicit BoatItem(Boat::Type type);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);

private:
    Boat::Type mType;
};

#endif
