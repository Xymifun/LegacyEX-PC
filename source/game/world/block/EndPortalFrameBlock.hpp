#ifndef GAME_WORLD_BLOCK_ENDPORTALFRAMEBLOCK_HPP
#define GAME_WORLD_BLOCK_ENDPORTALFRAMEBLOCK_HPP

#include "game/world/block/Block.hpp"

class AABB;
class BlockPattern;
class BooleanProperty;
class DirectionProperty;

class EndPortalFrameBlock : public Block
{
public:
    static DirectionProperty* FACING;
    static BooleanProperty* EYE;

    EndPortalFrameBlock();

    static void blockStaticCtor();
    virtual bool isSolidRender(const BlockState* state) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isCubeShaped(const BlockState* state);
    static BlockPattern* getOrCreatePortalShape();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    TextureAtlasSprite* getEye();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    TextureAtlasSprite* mTopIcon;
    TextureAtlasSprite* mEyeIcon;
};

#endif
