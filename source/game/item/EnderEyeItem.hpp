#ifndef GAME_ITEM_ENDEREYEITEM_HPP
#define GAME_ITEM_ENDEREYEITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"

class BlockPos;
class Direction;
class ItemTooltipDataHolder;
class Level;
class Player;

class EnderEyeItem : public Item
{
public:
    EnderEyeItem();
    virtual unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    virtual InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player,
                                        InteractionHand::EInteractionHand hand);
};

#endif
