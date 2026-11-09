#ifndef GAME_WORLD_BLOCK_CORALBLOCK_HPP
#define GAME_WORLD_BLOCK_CORALBLOCK_HPP

#include "game/world/block/Block.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class Random;
class TextureAtlasSprite;

template <class T> class AbstractProperty;

class CoralBlock : public Block
{
public:
    CoralBlock();

    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual const BlockState* setPropertyValueFromString(const BlockState* state, const Property* property,
                                                         const std::wstring& value);
    virtual int getDescriptionId(int auxValue);
    virtual int getUseDescriptionId(int auxValue);
    virtual int getTickDelay(Level* level);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    bool isValidAuxValue(int auxValue);

    static AbstractProperty<int>* VARIANT;
    static AbstractProperty<bool>* DEAD;

private:
    TextureAtlasSprite* mIcons[10];
};


#endif
