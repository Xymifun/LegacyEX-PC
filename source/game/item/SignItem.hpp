#ifndef GAME_ITEM_SIGNITEM_HPP
#define GAME_ITEM_SIGNITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"

class BlockPos;
class Direction;
class Level;
class Player;

class SignItem : public Item
{
public:
    SignItem();

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
};

#endif
