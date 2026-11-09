#ifndef GAME_WORLD_BLOCK_PISTONMAGICBLOCK_HPP
#define GAME_WORLD_BLOCK_PISTONMAGICBLOCK_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BaseEntityBlock.hpp"
#include "game/world/block/EntityBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class CompoundTag;
class Direction;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class Mirror;
class PistonMovingBlockEntity;
class Random;
class Rotation;
class Vec3;
class Player;

class PistonMagicBlock : public BaseEntityBlock
{
public:
    static AbstractProperty<const Direction*>* FACING;
    static AbstractProperty<PistonHeadBlock::PistonType*>* TYPE;

    PistonMagicBlock();

    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static boost::shared_ptr<BlockEntity> newMovingBlockEntity(const BlockState* state, const Direction* direction,
                                                               bool extending, bool source,
                                                               CompoundTag* blockEntityTag);
    static void blockStaticCtor();

    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual void destroy(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    boost::shared_ptr<PistonMovingBlockEntity> getEntity(LevelSource* levelSource, const BlockPos& pos);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual HitResult* clip(const BlockState* state, Level* level, const BlockPos& pos, Vec3* from, Vec3* to);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
};

#endif
