#ifndef GAME_WORLD_BLOCK_GRASSPATHBLOCK_HPP
#define GAME_WORLD_BLOCK_GRASSPATHBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/item/ItemInstance.hpp"

class BlockPos;
class BlockState;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

class GrassPathBlock : public Block
{
public:
    GrassPathBlock();

    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    void updateBlockState(Level* level, const BlockPos& pos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isSilkTouchable();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                           const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    TextureAtlasSprite* mTopIcon;
};

#endif
