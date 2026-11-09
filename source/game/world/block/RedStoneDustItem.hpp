#ifndef GAME_WORLD_BLOCK_REDSTONEDUSTITEM_HPP
#define GAME_WORLD_BLOCK_REDSTONEDUSTITEM_HPP

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"

class BlockPos;
class Direction;
class Level;
class Player;

class RedStoneDustItem : public Item
{
public:
    RedStoneDustItem();
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
};

#endif
