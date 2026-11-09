#ifndef GAME_WORLD_BLOCK_CHORUSPLANTBLOCK_HPP
#define GAME_WORLD_BLOCK_CHORUSPLANTBLOCK_HPP

#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Level;
class LevelSource;
class Random;

class ChorusPlantBlock : public Block
{
public:
    static AbstractProperty<bool>* NORTH_PROPERTY;
    static AbstractProperty<bool>* EAST_PROPERTY;
    static AbstractProperty<bool>* SOUTH_PROPERTY;
    static AbstractProperty<bool>* WEST_PROPERTY;
    static AbstractProperty<bool>* UP_PROPERTY;
    static AbstractProperty<bool>* DOWN_PROPERTY;

    ChorusPlantBlock();
    virtual void DerivedInit();
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getRenderLayer();
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    const AABB* getVerticalShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    const AABB* getHorizontalShape(const BlockState* state, const Direction* direction);
    const AABB* getRandomNoduleShape(bool large, const BlockState* state, const Direction* direction);
    virtual bool isWaterBlocking() const;
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);
    virtual BlockStateDefinition* createBlockStateDefinition();
};

#endif
