#ifndef GAME_ITEM_WRITABLEBOOKITEM_HPP
#define GAME_ITEM_WRITABLEBOOKITEM_HPP

#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"

class CompoundTag;
class ItemTooltipDataHolder;
class Level;
class Player;

class WritableBookItem : public Item
{
public:
    WritableBookItem();

    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    static bool makeSureTagIsValid(CompoundTag* tag);
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
};

#endif
