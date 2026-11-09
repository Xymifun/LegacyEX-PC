#ifndef GAME_WORLD_BLOCK_CAULDRONBLOCK_HPP
#define GAME_WORLD_BLOCK_CAULDRONBLOCK_HPP

#include "game/world/block/BaseEntityBlock.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Entity;
class IconRegister;
class Item;
class ItemInstance;
class Level;
class LevelSource;
class Player;
class Random;
class TextureAtlasSprite;

class CauldronBlock : public BaseEntityBlock
{
public:
    static const std::wstring TEXTURE_INNER;
    static const std::wstring TEXTURE_BOTTOM;
    static const std::wstring TEXTURE_WATER;
    static AbstractProperty<int>* LEVEL;

    CauldronBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level*, int);
    virtual void addCollisionAABBs(const BlockState*, Level*, const BlockPos&, const AABB*, std::vector<AABB*>*,
                                   boost::shared_ptr<Entity>, bool);
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual bool isSolidRender(const BlockState*) const;
    virtual bool isCubeShaped(const BlockState*);
    virtual void entityInside(Level*, const BlockPos&, const BlockState*, const boost::shared_ptr<Entity>&);
    virtual bool TestUse();
    void useInventory(boost::shared_ptr<Player>, not_null_ptr<ItemInstance>, not_null_ptr<ItemInstance>, int);
    void spawnCauldronEvent(Level*, const BlockState*, const BlockPos&, int) const;
    void explodeCauldronContents(Level*, const BlockState*, const BlockPos&);
    void decrementStack(not_null_ptr<ItemInstance>);
    virtual bool use(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<Player>,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool);
    virtual void setWaterLevel(Level*, const BlockPos&, const BlockState*, int);
    virtual void handleRain(Level*, const BlockPos&);
    virtual Item* getResource(const BlockState*, Random*, int);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level*, const BlockPos&, const BlockState*);
    virtual bool hasAnalogOutputSignal(const BlockState*);
    virtual int getAnalogOutputSignal(const BlockState*, Level*, const BlockPos&);
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder&);
    virtual bool canProvideSupport(const BlockState*, const Direction*, EBlockSupportType);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isPathfindable(LevelSource*, const BlockPos&);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*);
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual void registerIcons(IconRegister*);
    static TextureAtlasSprite* getTexture(const std::wstring&);
    virtual EBlockRenderShape getRenderShape(const BlockState*);
    static void spawnPotionParticles(Level*, double, double, double, Random*, int, int);
    virtual void animateTick(const BlockState*, Level*, const BlockPos&, Random*);
    static void spawnSplashParticles(Level*, double, double, double, Random*, int, int);
    static void spawnBubbleParticles(Level*, double, double, double, Random*, int, int);
    static void spawnExplosionParticles(Level*, double, double, double, Random*, int, int);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState*);
    virtual bool ParticlesSurviveWithin();

private:
    TextureAtlasSprite* mInnerIcon;
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mBottomIcon;
    TextureAtlasSprite* mWaterIcon;
};

#endif
