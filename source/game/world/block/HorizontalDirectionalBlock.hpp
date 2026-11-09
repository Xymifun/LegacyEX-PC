#ifndef GAME_WORLD_BLOCK_HORIZONTALDIRECTIONALBLOCK_HPP
#define GAME_WORLD_BLOCK_HORIZONTALDIRECTIONALBLOCK_HPP

#include "game/world/block/Block.hpp"

class DirectionProperty;

class HorizontalDirectionalBlock : public Block
{
public:
    HorizontalDirectionalBlock(Material* material);
    HorizontalDirectionalBlock(Material* material, const MaterialColor* materialColor);
    virtual ~HorizontalDirectionalBlock() {}

    static void blockStaticCtor();

    static DirectionProperty* FACING;
    static DirectionProperty* FACING_PROPERTY;
};

#endif
