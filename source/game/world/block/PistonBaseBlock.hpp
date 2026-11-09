#ifndef GAME_WORLD_BLOCK_PISTONBASEBLOCK_HPP
#define GAME_WORLD_BLOCK_PISTONBASEBLOCK_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/DirectionalBlock.hpp"

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Mirror;
class Random;
class Rotation;
class TextureAtlasSprite;
class LivingEntity;
class Entity;
class Block;
class Property;
class ItemInstance;
template <typename T> class AbstractProperty;

class PistonBaseBlock : public DirectionalBlock
{
public:
    static AbstractProperty<const Direction*>* FACING;
    static AbstractProperty<bool>* EXTENDED;
    static const std::wstring TEXTURE_SIDE;
    static const std::wstring TEXTURE_TOP;
    static const std::wstring TEXTURE_TOP_STICKY;
    static const std::wstring TEXTURE_BOTTOM;
    static const std::wstring TEXTURE_INNER_TOP;
    static const float PLATFORM_THICKNESS;

    PistonBaseBlock(bool sticky);

    static void blockStaticCtor();
    virtual bool isViewBlocking(const BlockState* state);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isTopSolidBlocking(const BlockState* state);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual bool isSolidRender(const BlockState* state) const;
    int getNeighborSignal(Level* level, const BlockPos& pos, const Direction* direction);
    static bool IsPushRestrictedByLevelHeight(Level* level, const BlockPos& pos, const Direction* direction);
    static int isPushable(const BlockState* state, Level* level, const BlockPos& pos, const Direction* direction,
                          bool allowDestroy, const Direction* motionDirection);
    void checkIfExtend(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    static const Direction* getFacingFromPlayer(Level* level, const BlockPos& pos,
                                                boost::shared_ptr<LivingEntity> placer);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual bool canContainLiquid(const BlockState* state) const;
    int moveBlocks(Level* level, const BlockPos& pos, const Direction* direction, bool extending);
    virtual bool triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam);
    virtual bool isCubeShaped(const BlockState* state);
    static const Direction* getFacing(int data);
    virtual const BlockState* getBlockState(int data);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    static TextureAtlasSprite* getTexture(const std::wstring& name);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);

private:
    bool mSticky;
    TextureAtlasSprite* mInnerTopIcon;
    TextureAtlasSprite* mBottomIcon;
    TextureAtlasSprite* mTopIcon;
};

#endif
