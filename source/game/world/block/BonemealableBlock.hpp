#pragma once

class BlockPos;
class BlockState;
class Level;
class Random;

class BonemealableBlock
{
public:
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state,
                                       bool isClientSide) = 0;
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state) = 0;
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state) = 0;
};
