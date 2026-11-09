#ifndef GAME_WORLD_BLOCK_WEIGHTEDPRESSUREPLATEBLOCK_HPP
#define GAME_WORLD_BLOCK_WEIGHTEDPRESSUREPLATEBLOCK_HPP

#include <string>

#include "game/world/block/BasePressurePlateBlock.hpp"

class BlockPos;
class BlockState;
class BlockStateDefinition;
class Level;
class Material;
class MaterialColor;
template <class T> class AbstractProperty;

class WeightedPressurePlateBlock : public BasePressurePlateBlock
{
public:
    static AbstractProperty<int>* POWER;

    WeightedPressurePlateBlock(const std::wstring& name, Material* material, int maxWeight,
                               const MaterialColor* materialColor);

    virtual int getSignalStrength(Level* level, const BlockPos& pos);
    virtual void playOnSound(Level* level, const BlockPos& pos);
    virtual void playOffSound(Level* level, const BlockPos& pos);
    virtual int getSignalForState(const BlockState* state);
    virtual const BlockState* setSignalForState(const BlockState* state, int signal);
    virtual int getTickDelay(Level* level);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();

private:
    int mMaxWeight;
};

#endif
