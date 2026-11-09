#ifndef GAME_WORLD_BLOCK_VINEBLOCK_HPP
#define GAME_WORLD_BLOCK_VINEBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/BooleanProperty.hpp"

template <class T> class AbstractProperty;

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Level;
class LevelSource;
class Mirror;
class Random;
class Rotation;

class VineBlock : public Block
{
public:
    static BooleanProperty* UP;
    static BooleanProperty* NORTH;
    static BooleanProperty* EAST;
    static BooleanProperty* SOUTH;
    static BooleanProperty* WEST;
    static BooleanProperty* ALL_FACES[5];
    static const AABB* UP_AABB;
    static const AABB* WEST_AABB;
    static const AABB* EAST_AABB;
    static const AABB* NORTH_AABB;
    static const AABB* SOUTH_AABB;

    VineBlock();
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    static bool isExceptionBlockForAttaching(Block* block);
    bool isAcceptableNeighbor(Level* level, const BlockPos& pos, const Direction* direction);
    bool canAttachBelow(Block* block);
    bool canAttachTo(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    static AbstractProperty<bool>* getPropertyForFace(const Direction* direction);
    static int countFaces(const BlockState* state);
    bool updateSurvival(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getColor() const;
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
};

#endif
