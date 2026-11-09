#ifndef GAME_WORLD_BLOCK_BLOCKIMPL_HPP
#define GAME_WORLD_BLOCK_BLOCKIMPL_HPP

#include "game/world/block/Block.hpp"

class Material;
class MaterialColor;

class BlockImpl : public Block
{
public:
    BlockImpl(Material* material);
    BlockImpl(Material* material, const MaterialColor* materialColor);
};

#endif
