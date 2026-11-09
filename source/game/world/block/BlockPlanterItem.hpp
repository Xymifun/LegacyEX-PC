#ifndef GAME_WORLD_BLOCK_BLOCKPLANTERITEM_HPP
#define GAME_WORLD_BLOCK_BLOCKPLANTERITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/item/Item.hpp"

class Block;
class BlockPos;
class Direction;
class Level;
class Player;

class BlockPlanterItem : public Item
{
public:
    BlockPlanterItem(Block* block);

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);

private:
    Block* mBlock;
};

#endif
