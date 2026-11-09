#ifndef GAME_CORE_CORALFANHANG_HPP
#define GAME_CORE_CORALFANHANG_HPP

#include "game/core/CoralFan.hpp"

template <class T> class AbstractProperty;

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class Level;
class LevelSource;
class Random;

class CoralFanHang : public CoralFan
{
public:
    CoralFanHang();

    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    const Direction* _getNeighborFromDirection(int direction) const;
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void checkAlive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);

    static AbstractProperty<int>* VARIANT;
    static AbstractProperty<int>* DIRECTION;
    static AbstractProperty<bool>* DEAD;
};


#endif
