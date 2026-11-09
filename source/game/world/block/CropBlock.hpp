#ifndef GAME_WORLD_BLOCK_CROPBLOCK_HPP
#define GAME_WORLD_BLOCK_CROPBLOCK_HPP

#include "game/core/Bush.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Item;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

template <typename T> class AbstractProperty;

class CropBlock : public Bush, public BonemealableBlock
{
public:
    static AbstractProperty<int>* AGE;

    explicit CropBlock(bool derivedInit = false);
    virtual ~CropBlock() {}

    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual void DerivedInit();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual AbstractProperty<int>* getAgeProperty();
    virtual int getMaxAge();
    virtual void growCrops(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getBonemealAgeIncrease(Level* level);
    virtual Item* getBaseSeedId();
    virtual Item* getBasePlantId();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    int getAge(const BlockState* state);
    const BlockState* getStateForAge(int age);
    bool isMaxAge(const BlockState* state);
    static float getGrowthSpeed(Block* block, Level* level, const BlockPos& pos);

protected:
    TextureAtlasSprite* mStageIcons[8];
};

#endif
