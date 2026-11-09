#ifndef GAME_ITEM_SNOWITEM_HPP
#define GAME_ITEM_SNOWITEM_HPP

#include "game/world/block/BlockItem.hpp"

class Block;
class BlockPos;
class Direction;

class SnowItem : public BlockItem
{
public:
    explicit SnowItem(Block* block);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual int getLevelDataForAuxValue(int auxValue);
};

#endif
