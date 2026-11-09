#ifndef GAME_WORLD_BLOCK_CORALFANBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_CORALFANBLOCKITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockItem.hpp"

class Block;
class BlockPos;
class Direction;
class ItemInstance;
class Level;
class Player;

class CoralFanBlockItem : public BlockItem
{
public:
    explicit CoralFanBlockItem(Block* block);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual int getLevelDataForAuxValue(int auxValue);

private:
    bool mIsDead;
};

#endif
