#ifndef GAME_WORLD_BLOCK_FARMBLOCK_HPP
#define GAME_WORLD_BLOCK_FARMBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Entity;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;

template <class T> class AbstractProperty;

class FarmBlock : public Block
{
public:
    static AbstractProperty<int>* MOISTURE;

    FarmBlock();
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual bool isSolidRender(const BlockState*) const;
    virtual bool isCubeShaped(const BlockState*);
    void turnToDirt(Level*, const BlockPos&);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual void fallOn(Level*, const BlockPos&, boost::shared_ptr<Entity>, float);
    virtual bool isUnderCrops(Level*, const BlockPos&);
    virtual bool isNearWater(Level*, const BlockPos&);
    virtual void neighborChanged(const BlockState*, Level*, const BlockPos&, Block*, const BlockPos&);
    virtual void onPlace(Level*, const BlockPos&, const BlockState*);
    virtual bool shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*);
    virtual Item* getResource(const BlockState*, Random*, int);
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual bool canContainLiquid(const BlockState*) const;
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level*, const BlockPos&, const BlockState*);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*);
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual void registerIcons(IconRegister*);

private:
    TextureAtlasSprite* mWetIcon;
    TextureAtlasSprite* mDryIcon;
};

#endif
