#ifndef GAME_WORLD_BLOCK_DIRECTIONALBLOCK_HPP
#define GAME_WORLD_BLOCK_DIRECTIONALBLOCK_HPP

#include "game/world/block/Block.hpp"

class DirectionProperty;

class DirectionalBlock : public Block
{
public:
    static DirectionProperty* FACING;
    static DirectionProperty* FACING_PROPERTY;

    DirectionalBlock(Material* material);
    DirectionalBlock(Material* material, const MaterialColor* materialColor);

    virtual ~DirectionalBlock() {}

    static void blockStaticCtor();
};

#endif
