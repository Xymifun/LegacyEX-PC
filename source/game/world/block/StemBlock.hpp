#ifndef GAME_WORLD_BLOCK_STEMBLOCK_HPP
#define GAME_WORLD_BLOCK_STEMBLOCK_HPP

#include "game/core/Bush.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class AABB;
class Block;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class DirectionProperty;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

class StemBlock : public Bush, public BonemealableBlock
{
public:
    static AbstractProperty<int>* AGE;
    static DirectionProperty* FACING;

    explicit StemBlock(Block* fruitType = NULL);

    virtual void growCrops(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);

    static void blockStaticCtor();

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    Item* getSeedItem();
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void registerIcons(IconRegister* iconRegister);

    TextureAtlasSprite* getAngledTexture();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mAngledTexture;
    Block* mFruitType;
};

#endif
