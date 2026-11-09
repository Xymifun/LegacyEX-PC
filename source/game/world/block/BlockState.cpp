#include "game/world/block/BlockState.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"

boost::shared_ptr<ItemInstance> createItem(const BlockState* state)
{
    Block* block = state->getBlock();
    return boost::shared_ptr<ItemInstance>(new ItemInstance(block, 1, block->convertBlockStateToLegacyData(state)));
}
