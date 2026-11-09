#ifndef GAME_ITEM_ENDERPEARLITEM_HPP
#define GAME_ITEM_ENDERPEARLITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"

class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class Player;

class EnderpearlItem : public Item
{
public:
    EnderpearlItem();
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
