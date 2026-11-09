#ifndef GAME_ITEM_DOORITEM_HPP
#define GAME_ITEM_DOORITEM_HPP

#include "game/item/Item.hpp"

class Block;
class BlockPos;
class Direction;
class Level;
class Player;

class DoorItem : public Item
{
public:
    DoorItem(Block* block);
    static void place(Level* level, const BlockPos& pos, const Direction* direction, Block* block, bool hingeRight);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    Block* mBlock;
};

#endif
