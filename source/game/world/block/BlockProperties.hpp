#ifndef GAME_WORLD_BLOCK_BLOCKPROPERTIES_HPP
#define GAME_WORLD_BLOCK_BLOCKPROPERTIES_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

class AABB;
class BlockPos;
class BlockState;
class Direction;
class Entity;
class HitResult;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Mirror;
class Player;
class Rotation;
class Vec3;

enum EBlockSupportType
{
    BLOCK_SUPPORT_CENTER = 0,
    BLOCK_SUPPORT_EDGE = 1,
    BLOCK_SUPPORT_ANY = 2
};

enum EPistonPushReaction
{
    PISTON_PUSH_NORMAL = 0,
    PISTON_PUSH_DESTROY = 1,
    PISTON_PUSH_BLOCK = 2,
    PISTON_PUSH_IGNORE = 3,
    PISTON_PUSH_PUSH_ONLY = 4
};

enum EBlockFaceShape
{
    BLOCK_FACE_SOLID = 0,
    BLOCK_FACE_BOWL = 1,
    BLOCK_FACE_CENTER_SMALL = 2,
    BLOCK_FACE_MIDDLE_POLE_THIN = 3,
    BLOCK_FACE_CENTER = 4,
    BLOCK_FACE_MIDDLE_POLE = 5,
    BLOCK_FACE_CENTER_BIG = 6,
    BLOCK_FACE_MIDDLE_POLE_THICK = 7,
    BLOCK_FACE_UNDEFINED = 8
};

class BlockProperties
{
public:
    virtual Material* getMaterial() const = 0;
    virtual bool isSolid() const = 0;
    virtual bool isValidSpawn(boost::shared_ptr<Entity> entity) const = 0;
    virtual int getLightBlock() const = 0;
    virtual int getLightEmission() const = 0;
    virtual bool isTranslucent() const = 0;
    virtual bool isTransparent() const = 0;
    virtual bool doesPropagate() const = 0;
    virtual const MaterialColor* getMapColor(LevelSource* levelSource, const BlockPos& pos) = 0;
    virtual const BlockState* rotate(Rotation* rotation) const = 0;
    virtual const BlockState* mirror(Mirror* mirror) const = 0;
    virtual bool isCubeShaped() const = 0;
    virtual bool hasCustomBreakingProgress() const = 0;
    virtual int getRenderShape() const = 0;
    virtual int getLightColor(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual float getShadeBrightness() const = 0;
    virtual bool isSolidBlockingCube() const = 0;
    virtual bool isSolidBlockingCubeAndNotSignalSource() const = 0;
    virtual bool isSolidBlockingBlock() const = 0;
    virtual bool canProvideSupport(const Direction* direction, EBlockSupportType supportType) const = 0;
    virtual bool isSignalSource() const = 0;
    virtual int getSignal(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual bool hasAnalogOutputSignal() const = 0;
    virtual int getAnalogOutputSignal(Level* level, const BlockPos& pos) const = 0;
    virtual float getDestroySpeed(Level* level, const BlockPos& pos) const = 0;
    virtual float getDestroyProgress(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos) const = 0;
    virtual int getDirectSignal(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual EPistonPushReaction getPistonPushReaction() const = 0;
    virtual const BlockState* fillVirtualBlockStateProperties(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual const AABB* getOutlineAABB(Level* level, const BlockPos& pos) const = 0;
    virtual bool shouldRenderFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction) const = 0;
    virtual bool isSolidRender() const = 0;
    virtual const AABB* getClipAABB(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual void addCollissionAABBs(Level* level, const BlockPos& pos, const AABB* entityBox,
                                    std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                    bool isActualState) const = 0;
    virtual const AABB* getShape(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual HitResult* clip(Level* level, const BlockPos& pos, Vec3* from, Vec3* to) const = 0;
    virtual bool isTopSolidBlocking() const = 0;
    virtual Vec3* getOffset(LevelSource* levelSource, const BlockPos& pos) const = 0;
    virtual bool isViewBlocking() const = 0;
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockPos& pos,
                                              const Direction* direction) const = 0;
};

#endif
