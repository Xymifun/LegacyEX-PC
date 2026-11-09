#ifndef GAME_WORLD_BLOCK_PRESSUREPLATEBLOCK_HPP
#define GAME_WORLD_BLOCK_PRESSUREPLATEBLOCK_HPP

#include <string>

#include "game/world/block/BasePressurePlateBlock.hpp"

class BlockState;
class BlockStateDefinition;
class BlockPos;
class Level;
class Material;
template <class T> class AbstractProperty;

class PressurePlateBlock : public BasePressurePlateBlock
{
public:
    enum Sensitivity
    {
        EVERYTHING,
        MOBS
    };

    static AbstractProperty<bool>* POWERED;

    PressurePlateBlock(const std::wstring& name, Material* material, Sensitivity sensitivity);
    virtual int getSignalForState(const BlockState* state);
    virtual const BlockState* setSignalForState(const BlockState* state, int signal);
    virtual void playOnSound(Level* level, const BlockPos& pos);
    virtual void playOffSound(Level* level, const BlockPos& pos);
    virtual int getSignalStrength(Level* level, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();

private:
    Sensitivity mSensitivity;
};

#endif
