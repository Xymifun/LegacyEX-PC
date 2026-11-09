#ifndef GAME_WORLD_BLOCK_REDSTONEOREBLOCK_HPP
#define GAME_WORLD_BLOCK_REDSTONEOREBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Direction;
class Level;
class Random;

class RedStoneOreBlock : public Block
{
public:
    RedStoneOreBlock(bool lit);

    virtual int getTickDelay(Level* level);
    virtual void attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player);
    virtual void stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity);
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    void spawnParticles(Level* level, const BlockPos& pos);
    virtual void interact(Level* level, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
    virtual int getResourceCount(Random* random);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);

private:
    bool mLit;
};

#endif
