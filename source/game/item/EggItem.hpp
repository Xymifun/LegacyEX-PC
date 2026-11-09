#ifndef GAME_ITEM_EGGITEM_HPP
#define GAME_ITEM_EGGITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"

class ItemTooltipDataHolder;
class Level;
class Player;

class EggItem : public Item
{
public:
    EggItem();
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
};

#endif
