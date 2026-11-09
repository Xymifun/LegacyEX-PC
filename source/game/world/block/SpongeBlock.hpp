#ifndef GAME_WORLD_BLOCK_SPONGEBLOCK_HPP
#define GAME_WORLD_BLOCK_SPONGEBLOCK_HPP

#include "game/world/block/Block.hpp"

template <typename T>
class AbstractProperty;

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Direction;
class IconRegister;
class Level;
class Random;
class TextureAtlasSprite;

class SpongeBlock : public Block
{
public:
    static AbstractProperty<bool>* WET;

    SpongeBlock();

    virtual std::wstring getName();
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    int removeWaterBreadthFirstSearch(Level* level, const BlockPos& pos);
    void tryAbsorbWater(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);

private:
    TextureAtlasSprite* mWetIcon;
};

#endif
