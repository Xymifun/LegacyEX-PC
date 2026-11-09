#ifndef GAME_WORLD_BLOCK_BASEPRESSUREPLATEBLOCK_HPP
#define GAME_WORLD_BLOCK_BASEPRESSUREPLATEBLOCK_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/world/block/Block.hpp"

class AABB;
class BlockPos;
class BlockState;
class Direction;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Random;

class BasePressurePlateBlock : public Block
{
public:
    BasePressurePlateBlock(const std::wstring& name, Material* material);
    BasePressurePlateBlock(const std::wstring& name, Material* material, const MaterialColor* materialColor);
    virtual ~BasePressurePlateBlock() {}

    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getTickDelay(Level* level);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool blocksLight();
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isPossibleToRespawnInThis();
    bool canPlaceOn(Level* level, const BlockPos& pos);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual void checkPressed(Level* level, const BlockPos& pos, const BlockState* state, int signal);

    virtual void playOnSound(Level* level, const BlockPos& pos) = 0;
    virtual void playOffSound(Level* level, const BlockPos& pos) = 0;
    virtual const AABB* getSensitiveAABB(const BlockPos& pos);
    virtual void updateNeighbours(Level* level, const BlockPos& pos);
    virtual int getSignalStrength(Level* level, const BlockPos& pos) = 0;
    virtual int getSignalForState(const BlockState* state) = 0;
    virtual const BlockState* setSignalForState(const BlockState* state, int signal) = 0;

    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual bool isSignalSource(const BlockState* state);
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual void registerIcons(IconRegister* iconRegister);

protected:
    std::wstring mTextureName;
};

#endif
