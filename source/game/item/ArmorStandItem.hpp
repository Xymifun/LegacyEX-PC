#ifndef GAME_ITEM_ARMORSTANDITEM_HPP
#define GAME_ITEM_ARMORSTANDITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"

class BlockPos;
class Direction;
class Level;
class Player;

class ArmorStandItem : public Item
{
public:
    ArmorStandItem();

    virtual ~ArmorStandItem();
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
};

#endif
