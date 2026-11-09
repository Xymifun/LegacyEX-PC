#ifndef GAME_WORLD_BLOCK_CAKEBLOCK_HPP
#define GAME_WORLD_BLOCK_CAKEBLOCK_HPP

#include "game/world/block/Block.hpp"

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
class Player;

class CakeBlock : public Block
{
public:
    CakeBlock();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool TestUse();
    bool eat(Level* level, const BlockPos& pos, const BlockState* state, const boost::shared_ptr<Player>& player);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getResourceCount(Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mIcons[3];
};

#endif
