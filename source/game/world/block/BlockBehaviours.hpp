#ifndef GAME_WORLD_BLOCK_BLOCKBEHAVIOURS_HPP
#define GAME_WORLD_BLOCK_BLOCKBEHAVIOURS_HPP

class Block;
class BlockPos;
class Level;

class BlockBehaviours
{
public:
    virtual bool triggerEvent(Level* level, const BlockPos& pos, int eventId, int eventParam) const = 0;
    virtual void neighborChanged(Level* level, const BlockPos& pos, Block* block, const BlockPos& fromPos) const = 0;
};

#endif
