#ifndef GAME_WORLD_BLOCK_STAIRBLOCK_HPP
#define GAME_WORLD_BLOCK_STAIRBLOCK_HPP

#include "game/world/block/Block.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/core/Direction.hpp"
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class AABB;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Explosion;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Mirror;
class Random;
class Rotation;
class TextureAtlasSprite;
class Vec3;

class DirectionProperty;

class StairBlock : public Block
{
public:
    class Half : public StringRepresentable
    {
    public:
        static Half* TOP;
        static Half* BOTTOM;

        Half(const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring mName;
    };
    class Shape : public StringRepresentable
    {
    public:
        Shape(const std::wstring& name, int id);
        int getId() const;
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring mName;
        int mId;
    };
    static DirectionProperty* FACING;
    static EnumProperty<Half*>* HALF;
    static EnumProperty<Shape*>* SHAPE;
    StairBlock(const BlockState* modelState);
    static void blockStaticCtor();
    virtual void addCollisionAABBs(const BlockState*, Level*, const BlockPos&, const AABB*, std::vector<AABB*>*,
                                   boost::shared_ptr<Entity>, bool);
    static const AABB* getStepAABB(const BlockState*);
    static const AABB* getCornerAABB(const BlockState*);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*);
    virtual int getSpawnResourcesAuxValue(const BlockState*);
    virtual const arrayWithLength<const AABB*>* getShapes(const BlockState*, LevelSource*, const BlockPos&);
    virtual bool isSolidRender(const BlockState*) const;
    virtual bool isCubeShaped(const BlockState*);
    virtual void addLights(Level*, const BlockPos&);
    virtual void animateTick(const BlockState*, Level*, const BlockPos&, Random*);
    virtual void attack(Level*, const BlockPos&, boost::shared_ptr<Player>);
    virtual void destroy(Level*, const BlockPos&, const BlockState*);
    virtual int getLightColor(const BlockState*, LevelSource*, const BlockPos&, int);
    virtual float getExplosionResistance(boost::shared_ptr<Entity>);
    virtual int getRenderLayer();
    virtual int getTickDelay(Level*);
    virtual const AABB* getOutlineAABB(const BlockState*, Level*, const BlockPos&);
    virtual Vec3* handleEntityInside(Level*, const BlockPos&, boost::shared_ptr<Entity>, Vec3*);
    virtual bool mayPick();
    virtual bool mayPick(const BlockState*, bool);
    virtual bool mayPlace(Level*, const BlockPos&);
    virtual void onPlace(Level*, const BlockPos&, const BlockState*);
    virtual void onRemove(Level*, const BlockPos&, const BlockState*);
    virtual void prepareRender(Level*, const BlockPos&);
    virtual void stepOn(Level*, const BlockPos&, boost::shared_ptr<Entity>);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual bool use(Level*, const BlockPos&, const BlockState*, boost::shared_ptr<Player>,
                     InteractionHand::EInteractionHand, const Direction*, float, float, float, bool);
    virtual void wasExploded(Level*, const BlockPos&, Explosion*);
    virtual bool isTopSolidBlocking(const BlockState*);
    virtual const MaterialColor* getMapColor(const BlockState*, LevelSource*, const BlockPos&);
    virtual const BlockState* getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float, float, int,
                                                   boost::shared_ptr<LivingEntity>);
    bool _neighboringBlockCheckForCreatingBarrierInDirection(LevelSource*, const BlockPos&, const BlockState*,
                                                             const Direction*) const;
    virtual bool liquidCanFlowIntoFromDirection(LevelSource*, const BlockPos&, const Direction*) const;
    using Block::clip;
    virtual HitResult* clip(const BlockState*, Level*, const BlockPos&, Vec3*, Vec3*);
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    static bool isStairs(const BlockState*);
    static bool canTakeShape(const BlockState*, LevelSource*, const BlockPos&, const Direction*);
    static Shape* getStairsShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState*, LevelSource*, const BlockPos&);
    static bool isStairs(int);
    virtual const BlockState* rotate(const BlockState*, Rotation*);
    virtual const BlockState* mirror(const BlockState*, Mirror*);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual void registerIcons(IconRegister*);
    virtual EBlockRenderShape getRenderShape(const BlockState*);

private:
    Block* mModelBlock;
    const BlockState* mModelState;
    bool mIsModelState;
    int mShapeState;
};

#endif
