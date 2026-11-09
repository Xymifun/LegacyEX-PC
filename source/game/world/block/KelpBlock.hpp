#ifndef GAME_WORLD_BLOCK_KELPBLOCK_HPP
#define GAME_WORLD_BLOCK_KELPBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/BonemealableBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;
template <class T> class AbstractProperty;

class KelpBlock : public Block, public BonemealableBlock
{
public:
    static AbstractProperty<int>* AGE;

    KelpBlock();

    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getRenderLayer();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool isSilkTouchable();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual std::wstring getTileItemIconName();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    void checkAlive(Level* level, const BlockPos& pos);
    int getAge(const BlockState* state);
    bool shouldGrow(Level* level, const BlockPos& pos);

    TextureAtlasSprite* mIcons[5];
};

#endif
