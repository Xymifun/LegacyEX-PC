#ifndef GAME_WORLD_BLOCK_REDSTONELAMPBLOCK_HPP
#define GAME_WORLD_BLOCK_REDSTONELAMPBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class IconRegister;
class Level;
class Random;

class RedstoneLampBlock : public Block
{
public:
    RedstoneLampBlock(bool lit);

    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    bool mLit;
};

#endif
