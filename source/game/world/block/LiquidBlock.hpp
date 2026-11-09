#ifndef GAME_WORLD_BLOCK_LIQUIDBLOCK_HPP
#define GAME_WORLD_BLOCK_LIQUIDBLOCK_HPP

#include "game/world/block/Block.hpp"

#include <string>

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Material;
class Random;
class TextureAtlasSprite;
class Vec3;

template <typename T> class AbstractProperty;

class LiquidBlock : public Block
{
public:
    static AbstractProperty<int>* LEVEL;
    static const std::wstring TEXTURE_LAVA_STILL;
    static const std::wstring TEXTURE_WATER_STILL;
    static const std::wstring TEXTURE_WATER_FLOW;
    static const std::wstring TEXTURE_LAVA_FLOW;

    LiquidBlock(Material* material);
    virtual ~LiquidBlock() {}

    virtual bool isLiquidBlock();
    static float getHeightFromData(int data);
    static unsigned short getLiquidElement(LevelSource* levelSource, const BlockPos& pos);
    virtual void DerivedInit();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual bool canContainLiquid(const BlockState* state) const;
    int getDepth(LevelSource* levelSource, const BlockPos& pos);
    int getRenderedDepth(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPick(const BlockState* state, bool includeLiquid);
    virtual bool isSolidFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual bool shouldRenderBackwardUpFace(LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getResourceCount(Random* random);
    virtual Vec3* getFlow(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual Vec3* handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual int getTickDelay(Level* level);
    virtual int getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue);
    virtual int getRenderLayer();
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    static LiquidBlock* getDynamicLiquid(Material* material);
    static double getSlopeAngle(LevelSource* levelSource, const BlockPos& pos, Material* material,
                                const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual bool updateLiquid(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual void fizz(Level* level, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    static unsigned short getDynamicLiquidElementId(unsigned short elementId);
    static unsigned short getStaticLiquidElementId(unsigned short elementId);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    static float getBlockLiquidHeight(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    static float getLiquidHeight(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    static void setLiquidElement(Level* level, const BlockPos& pos, unsigned short elementId, bool extraLayer,
                                 int flags);
    void trySpreadFire(Level* level, const BlockPos& pos, Random* random);
    static LiquidBlock* getStaticLiquid(Material* material);
    static TextureAtlasSprite* getTexture(const std::wstring& name);

private:
    TextureAtlasSprite* mIcons[2];
};

#endif
