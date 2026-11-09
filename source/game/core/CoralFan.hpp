#ifndef GAME_CORE_CORALFAN_HPP
#define GAME_CORE_CORALFAN_HPP

#include "game/core/Bush.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class MaterialColor;
class Random;
class TextureAtlasSprite;

template <class T> class AbstractProperty;

class CoralFan : public Bush
{
public:
    CoralFan();
    virtual ~CoralFan() {}

    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getRenderLayer();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual EBlockOffsetType getOffsetType();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int getDescriptionId(int auxValue);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isSilkTouchable();
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool isWaterBlocking() const;
    virtual void checkAlive(Level* level, const BlockPos& pos, const BlockState* state);

protected:
    TextureAtlasSprite* mIcons[5];
    TextureAtlasSprite* mDeadIcons[5];

public:
    static AbstractProperty<int>* VARIANT;
    static AbstractProperty<int>* DIRECTION;
};


#endif
