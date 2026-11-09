#ifndef GAME_WORLD_BLOCK_GRASSBLOCK_HPP
#define GAME_WORLD_BLOCK_GRASSBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/BonemealableBlock.hpp"

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

class GrassBlock : public Block, public BonemealableBlock
{
public:
    static AbstractProperty<bool>* SNOWY;

    GrassBlock();

    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual int getColor() const;
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getRenderLayer();
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool waterSpreadCausesSpawn();
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                           const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    static TextureAtlasSprite* getSideTextureOverlay();

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mSnowSideIcon;
    TextureAtlasSprite* mSideOverlayIcon;

    bool randomWalk(Level* level, Random* random, BlockPos& pos, int distance);
};

#endif
