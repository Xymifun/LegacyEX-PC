#ifndef GAME_WORLD_BLOCK_SEAPICKLEBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_SEAPICKLEBLOCKITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/world/block/BlockItem.hpp"

class Block;
class BlockPos;
class Direction;
class Level;
class Player;

class SeaPickleBlockItem : public BlockItem
{
public:
    explicit SeaPickleBlockItem(Block* block);

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual int getLevelDataForAuxValue(int auxValue);
};

#endif
