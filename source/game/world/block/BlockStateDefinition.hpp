#ifndef GAME_WORLD_BLOCK_BLOCKSTATEDEFINITION_HPP
#define GAME_WORLD_BLOCK_BLOCKSTATEDEFINITION_HPP

#include <map>
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/AbstractBlockState.hpp"

class AABB;
class Block;
class BlockPos;
class Boxed;
class Direction;
class Entity;
class Level;
class LevelSource;
class Material;
class MaterialColor;
class Mirror;
class Player;
class Property;
class Rotation;
class Vec3;
class HitResult;

class BlockStateDefinition
{
public:
    class BlockStateImpl : public AbstractBlockState
    {
    public:
        BlockStateImpl(BlockStateDefinition* definition, unsigned short data);
        virtual bool triggerEvent(Level*, const BlockPos&, int, int) const;
        virtual void neighborChanged(Level*, const BlockPos&, Block*, const BlockPos&) const;
        bool valuesContainsKey(const Property*) const;
        virtual const std::vector<const Property*>& getProperties() const;
        virtual bool hasProperty(const Property*) const;
        virtual Boxed* getBoxedValue(const Property*) const;
        virtual const BlockState* setBoxedValue(const Property*, Boxed*) const;
        virtual boost::unordered_map<const Property*, Boxed*>* getValues() const;
        virtual Block* getBlock() const;
        virtual bool equals(const BlockState*);
        virtual int hashCode() const;
        virtual Material* getMaterial() const;
        virtual bool isSolid() const;
        virtual bool isValidSpawn(boost::shared_ptr<Entity>) const;
        virtual int getLightBlock() const;
        virtual int getLightEmission() const;
        virtual bool isTranslucent() const;
        virtual bool isTransparent() const;
        virtual bool doesPropagate() const;
        virtual const MaterialColor* getMapColor(LevelSource*, const BlockPos&);
        virtual const BlockState* rotate(Rotation*) const;
        virtual const BlockState* mirror(Mirror*) const;
        virtual bool isCubeShaped() const;
        virtual bool hasCustomBreakingProgress() const;
        virtual int getRenderShape() const;
        virtual int getLightColor(LevelSource*, const BlockPos&) const;
        virtual float getShadeBrightness() const;
        virtual bool isSolidBlockingCube() const;
        virtual bool isSolidBlockingCubeAndNotSignalSource() const;
        virtual bool isSolidBlockingBlock() const;
        virtual bool canProvideSupport(const Direction*, EBlockSupportType) const;
        virtual bool isSignalSource() const;
        virtual int getSignal(LevelSource*, const BlockPos&, const Direction*) const;
        virtual bool hasAnalogOutputSignal() const;
        virtual int getAnalogOutputSignal(Level*, const BlockPos&) const;
        virtual float getDestroySpeed(Level*, const BlockPos&) const;
        virtual float getDestroyProgress(boost::shared_ptr<Player>, Level*, const BlockPos&) const;
        virtual int getDirectSignal(LevelSource*, const BlockPos&, const Direction*) const;
        virtual EPistonPushReaction getPistonPushReaction() const;
        virtual const BlockState* fillVirtualBlockStateProperties(LevelSource*, const BlockPos&) const;
        virtual const AABB* getOutlineAABB(Level*, const BlockPos&) const;
        virtual bool shouldRenderFace(LevelSource*, const BlockPos&, const Direction*) const;
        virtual bool isSolidRender() const;
        virtual const AABB* getClipAABB(LevelSource*, const BlockPos&) const;
        virtual void addCollissionAABBs(Level*, const BlockPos&, const AABB*, std::vector<AABB*>*,
                                        boost::shared_ptr<Entity>, bool) const;
        virtual const AABB* getShape(LevelSource*, const BlockPos&) const;
        virtual HitResult* clip(Level*, const BlockPos&, Vec3*, Vec3*) const;
        virtual bool isTopSolidBlocking() const;
        virtual Vec3* getOffset(LevelSource*, const BlockPos&) const;
        virtual bool isViewBlocking() const;
        virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockPos&, const Direction*) const;

    public:
        Block* mBlock;
        BlockStateDefinition* mDefinition;
        unsigned short mData;
        const MaterialColor* mCachedMapColor;
    };

    explicit BlockStateDefinition(Block* block);
    BlockStateDefinition(Block* block, const Property** properties, unsigned int count)
    {
        _init(block, properties, count);
    }
    BlockStateImpl* any();
    const arrayWithLength<BlockStateImpl*>* getPossibleBlockStates() const;
    Block* getBlock() const;
    const std::vector<const Property*>& getProperties() const;
    std::vector<const std::vector<Boxed*>*>* getPropertyValues();
    void _init(Block*, const Property**, unsigned int);
    static std::wstring PROPERTY_TO_STRING_FUNCTION(const Property*);
    bool hasProperty(const Property*) const;
    Boxed* getPropertyValue(const Property*, unsigned short) const;
    BlockStateImpl* getNeighbour(const Property*, Boxed*, unsigned short) const;
    boost::unordered_map<const Property*, Boxed*>* getValues(unsigned short) const;
    std::wstring toString();

private:
    Block* mBlock;
    std::vector<const Property*> mProperties;
    arrayWithLength<BlockStateImpl*> mStates;
    unsigned char* mBitOffsets;
    unsigned short* mBitMasks;
};

#endif
