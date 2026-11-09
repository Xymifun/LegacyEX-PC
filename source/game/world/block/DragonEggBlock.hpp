#ifndef GAME_WORLD_BLOCK_DRAGONEGGBLOCK_HPP
#define GAME_WORLD_BLOCK_DRAGONEGGBLOCK_HPP

#include "game/world/block/Block.hpp"

#include <boost/shared_ptr.hpp>

class BlockPos;
class BlockState;
class Direction;
class Level;
class LevelSource;
class Player;
class Random;

class DragonEggBlock : public Block
{
public:
    DragonEggBlock();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    void checkSlide(Level* level, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    void teleport(Level* level, const BlockPos& pos);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player);
    virtual int getTickDelay(Level* level);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    static void generateTeleportParticles(Level* level, int x, int y, int z, int data);
};

#endif
