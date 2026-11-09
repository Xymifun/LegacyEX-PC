#ifndef GAME_CORE_CORAL_HPP
#define GAME_CORE_CORAL_HPP

#include "game/core/Bush.hpp"

template <class T> class AbstractProperty;

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

class Coral : public Bush
{
public:
    Coral();

    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual int getDescriptionId(int auxValue);
    virtual int getUseDescriptionId(int auxValue);
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual bool isSilkTouchable();
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);

private:
    TextureAtlasSprite* mIcons[5];

public:
    static AbstractProperty<int>* VARIANT;
};

#endif
