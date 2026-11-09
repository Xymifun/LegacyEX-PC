#ifndef GAME_WORLD_BLOCK_SLABBLOCKITEM_HPP
#define GAME_WORLD_BLOCK_SLABBLOCKITEM_HPP

#include "game/core/InteractionResult.hpp"
#include "game/world/block/BlockItem.hpp"

class Block;
class BlockPos;
class BlockState;
class Boxed;
class Direction;
class HalfSlabBlock;
class Level;
class Property;
class ItemInstance;
class Player;

class SlabBlockItem : public BlockItem
{
public:
    SlabBlockItem(Block* block, HalfSlabBlock* halfSlab, HalfSlabBlock* fullSlab);

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual int getLevelDataForAuxValue(int auxValue);
    virtual int getDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction,
                          boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance);

private:
    const BlockState* makeState(const Property* property, Boxed* value);
    bool tryConvertTargetBlock(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance, Level* level,
                               const BlockPos& pos, Boxed* type, bool simulate);

    HalfSlabBlock* mHalfSlab;
    HalfSlabBlock* mFullSlab;
};

#endif
