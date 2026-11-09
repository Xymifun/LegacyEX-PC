#ifndef GAME_WORLD_BLOCK_REDSTONETORCHBLOCK_HPP
#define GAME_WORLD_BLOCK_REDSTONETORCHBLOCK_HPP

#include "game/world/block/TorchBlock.hpp"
#include "game/world/block/BlockPos.hpp"

class Block;
class BlockState;
class Level;
class LevelSource;
class Random;
class Item;
class ItemInstance;

class RedstoneTorchBlock : public TorchBlock
{
public:
    class Toggle
    {
    public:
        Toggle(const BlockPos& pos, long long time)
        {
            this->pos = pos;
            this->time = time;
        }

        BlockPos pos;
        long long time;
    };

    RedstoneTorchBlock(bool lit);
    static void removeLevelReferences(Level* level);
    bool isToggledTooFrequently(Level* level, const BlockPos& pos, bool addToggle);
    virtual int getTickDelay(Level* level);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    bool hasNeighborSignal(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool isSignalSource(const BlockState* state);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isMatching(Block* block);

private:
    bool mLit;
};

#endif
