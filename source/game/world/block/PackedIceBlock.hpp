#ifndef GAME_WORLD_BLOCK_PACKEDICEBLOCK_HPP
#define GAME_WORLD_BLOCK_PACKEDICEBLOCK_HPP

#include "game/world/block/Block.hpp"

class Random;

class PackedIceBlock : public Block
{
public:
    PackedIceBlock();


    virtual int getResourceCount(Random* random);
};

#endif
