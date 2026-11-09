#ifndef GAME_WORLD_BLOCK_PUMPKINBLOCK_HPP
#define GAME_WORLD_BLOCK_PUMPKINBLOCK_HPP

#include "game/world/block/HorizontalDirectionalBlock.hpp"

class BlockPattern;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class Mirror;
class Rotation;
class TextureAtlasSprite;
class LivingEntity;

#include <boost/shared_ptr.hpp>

class PumpkinBlock : public HorizontalDirectionalBlock
{
public:
    PumpkinBlock();

    BlockPattern* getOrCreateSnowGolemFull();
    void fjAwardCraftedGolemStat(unsigned long long craftedEntity);
    BlockPattern* getOrCreateIronGolemFull();
    void trySpawnGolem(Level* level, const BlockPos& pos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    BlockPattern* getOrCreateSnowGolemBase();
    BlockPattern* getOrCreateIronGolemBase();
    bool canSpawnGolem(Level* level, const BlockPos& pos);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mFaceIcon;
    BlockPattern* mSnowGolemBasePattern;
    BlockPattern* mSnowGolemFullPattern;
    BlockPattern* mIronGolemBasePattern;
    BlockPattern* mIronGolemFullPattern;
};

#endif
