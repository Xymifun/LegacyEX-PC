#ifndef GAME_WORLD_BLOCK_COCOABLOCK_HPP
#define GAME_WORLD_BLOCK_COCOABLOCK_HPP

#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Mirror;
class Random;
class Rotation;
class TextureAtlasSprite;
class ItemInstance;
class LivingEntity;
template <typename T> class AbstractProperty;

class CocoaBlock : public HorizontalDirectionalBlock, public BonemealableBlock
{
public:
    static AbstractProperty<int>* AGE;
    CocoaBlock();
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    void removeAndSpawn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    using Block::spawnResources;
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    TextureAtlasSprite* getTextureForAge(int age);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mIcons[3];
};

#endif
